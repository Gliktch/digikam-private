#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2026 by the digiKam Private contributors
# SPDX-License-Identifier: BSD-3-Clause

set -euo pipefail

if (( $# != 1 )); then
    echo "Usage: $0 <digiKam AppImage>" >&2
    exit 2
fi

appimage=$(readlink -f "$1")

if [[ ! -x "$appimage" ]]; then
    echo "AppImage is not executable: $appimage" >&2
    exit 2
fi

for tool in xvfb-run xdotool xwd; do
    command -v "$tool" >/dev/null || {
        echo "Required smoke-test tool is missing: $tool" >&2
        exit 2
    }
done

fixture=$(mktemp -d "${TMPDIR:-/tmp}/digikam-private-ui-smoke.XXXXXX")

mkdir -m 700 \
    "$fixture/db" \
    "$fixture/config" \
    "$fixture/data" \
    "$fixture/cache" \
    "$fixture/runtime"

printf '%s\n' \
    '[General Settings]' \
    'Version=9.2.0' \
    '' \
    '[Album Settings]' \
    'Album Monitoring=false' > "$fixture/config/digikamrc"

export DIGIKAM_PRIVATE_SMOKE_APPIMAGE="$appimage"
export DIGIKAM_PRIVATE_SMOKE_FIXTURE="$fixture"

# shellcheck disable=SC2016
xvfb-run -a -s '-screen 0 1280x900x24' bash -c '
set -euo pipefail

app_pid=

cleanup()
{
    if [[ -n "$app_pid" ]] && kill -0 "$app_pid" 2>/dev/null; then
        kill -TERM "$app_pid" 2>/dev/null || true
        wait "$app_pid" 2>/dev/null || true
    fi
}

require_alive()
{
    if ! kill -0 "$app_pid" 2>/dev/null; then
        set +e
        wait "$app_pid"
        status=$?
        set -e
        app_pid=
        echo "digiKam exited unexpectedly with status $status" >&2
        exit 1
    fi
}

trap cleanup EXIT INT TERM

export HOME="$DIGIKAM_PRIVATE_SMOKE_FIXTURE"
export XDG_CONFIG_HOME="$DIGIKAM_PRIVATE_SMOKE_FIXTURE/config"
export XDG_DATA_HOME="$DIGIKAM_PRIVATE_SMOKE_FIXTURE/data"
export XDG_CACHE_HOME="$DIGIKAM_PRIVATE_SMOKE_FIXTURE/cache"
export XDG_RUNTIME_DIR="$DIGIKAM_PRIVATE_SMOKE_FIXTURE/runtime"
export LANG=C.UTF-8

"$DIGIKAM_PRIVATE_SMOKE_APPIMAGE" \
    --database-directory "$DIGIKAM_PRIVATE_SMOKE_FIXTURE/db" \
    --config "$DIGIKAM_PRIVATE_SMOKE_FIXTURE/config/digikamrc" \
    > "$DIGIKAM_PRIVATE_SMOKE_FIXTURE/app.log" 2>&1 &
app_pid=$!
printf "%s\n" "$app_pid" > "$DIGIKAM_PRIVATE_SMOKE_FIXTURE/app.pid"

configure_closed=0

for attempt in $(seq 1 200); do
    saw_digikam=0

    for candidate in $(xdotool search --onlyvisible --name "" 2>/dev/null); do
        name=$(xdotool getwindowname "$candidate" 2>/dev/null || true)

        case "$name" in
            Configure*)
                if (( configure_closed == 0 )); then
                    xdotool key --window "$candidate" Escape 2>/dev/null || true
                    configure_closed=1
                fi
                ;;
            digiKam)
                saw_digikam=1
                ;;
        esac
    done

    if (( configure_closed == 1 && saw_digikam == 1 )); then
        break
    fi

    require_alive
    sleep 0.1
done

main_window=

for attempt in $(seq 1 200); do
    for candidate in $(xdotool search --onlyvisible --name "" 2>/dev/null); do
        name=$(xdotool getwindowname "$candidate" 2>/dev/null || true)
        geometry=$(xdotool getwindowgeometry --shell "$candidate" 2>/dev/null || true)
        width=$(sed -n "s/^WIDTH=//p" <<< "$geometry")

        if [[ "$name" == digiKam ]] && [[ -n "$width" ]] && (( width < 700 )); then
            xdotool key --window "$candidate" Escape 2>/dev/null || true
        elif [[ "$name" == digiKam ]] && [[ -n "$width" ]] && (( width > 700 )); then
            main_window=$candidate
        fi
    done

    [[ -n "$main_window" ]] && break
    require_alive
    sleep 0.1
done

[[ -n "$main_window" ]]
sleep 2

for candidate in $(xdotool search --onlyvisible --name "" 2>/dev/null); do
    name=$(xdotool getwindowname "$candidate" 2>/dev/null || true)

    case "$name" in
        "Download Required Model Files"*)
            xdotool key --window "$candidate" Escape 2>/dev/null || true
            ;;
    esac
done

sleep 1
xdotool windowfocus --sync "$main_window"
xdotool key alt+s
sleep 1

# Privacy Categories is immediately above Configure digiKam in Settings.
xdotool key Up
sleep 0.5
xdotool key Up
sleep 0.5
xdotool key Return

privacy_window=

for attempt in $(seq 1 150); do
    for candidate in $(xdotool search --onlyvisible --name "" 2>/dev/null); do
        name=$(xdotool getwindowname "$candidate" 2>/dev/null || true)

        case "$name" in
            "Privacy Categories"*)
                privacy_window=$candidate
                ;;
        esac
    done

    [[ -n "$privacy_window" ]] && break
    require_alive
    sleep 0.1
done

[[ -n "$privacy_window" ]]
xwd -silent -id "$privacy_window" \
    -out "$DIGIKAM_PRIVATE_SMOKE_FIXTURE/privacy-dialog.xwd"
printf "privacy-dialog-title=%s\n" \
    "$(xdotool getwindowname "$privacy_window")"

# The superseded candidate opened the dialog, then aborted in its AI startup
# path. Keep it alive beyond that point before accepting the bundle.
for attempt in $(seq 1 30); do
    require_alive
    sleep 0.1
done

privacy_geometry=$(xdotool getwindowgeometry --shell "$privacy_window")
privacy_width=$(sed -n "s/^WIDTH=//p" <<< "$privacy_geometry")
privacy_height=$(sed -n "s/^HEIGHT=//p" <<< "$privacy_geometry")
xdotool mousemove --window "$privacy_window" \
    "$((privacy_width - 40))" "$((privacy_height - 24))"
xdotool click 1

for attempt in $(seq 1 50); do
    privacy_visible=0

    for candidate in $(xdotool search --onlyvisible --name "Privacy Categories" 2>/dev/null); do
        if [[ "$candidate" == "$privacy_window" ]]; then
            privacy_visible=1
        fi
    done

    if (( privacy_visible == 0 )); then
        break
    fi

    require_alive
    sleep 0.1
done

if (( privacy_visible != 0 )); then
    echo "Privacy Categories did not close" >&2
    exit 1
fi

xdotool windowfocus --sync "$main_window"
xdotool key ctrl+q

for attempt in $(seq 1 300); do
    if ! kill -0 "$app_pid" 2>/dev/null; then
        break
    fi

    sleep 0.1
done

if kill -0 "$app_pid" 2>/dev/null; then
    echo "digiKam did not exit after its main window closed" >&2

    for candidate in $(xdotool search --onlyvisible --name "" 2>/dev/null); do
        xdotool getwindowname "$candidate" 2>/dev/null || true
    done

    exit 1
fi

set +e
wait "$app_pid"
status=$?
set -e
app_pid=

if (( status != 0 )); then
    echo "digiKam exited with status $status" >&2
    exit 1
fi
'

printf 'privacy-category-ui-smoke=pass fixture=%s\n' "$fixture"
