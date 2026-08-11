#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2026 by the digiKam Private contributors
# SPDX-License-Identifier: BSD-3-Clause

set -euo pipefail

script_dir=$(cd -- "$(dirname -- "$0")" && pwd)
app_run=$(readlink -f "$script_dir/../data/AppRun")
fixture=$(mktemp -d "${TMPDIR:-/tmp}/digikam-private-profile-test.XXXXXX")

cleanup()
{
    rm -rf -- "$fixture"
}

trap cleanup EXIT INT TERM

install -d "$fixture/app/usr/bin" "$fixture/home"
install -m 755 "$app_run" "$fixture/app/AppRun"

cat > "$fixture/app/usr/bin/digikam" <<'EOF'
#!/usr/bin/env bash
printf '%s\n' \
    "config=$XDG_CONFIG_HOME" \
    "data=$XDG_DATA_HOME" \
    "cache=$XDG_CACHE_HOME" \
    "state=$XDG_STATE_HOME" \
    "database=$DIGIKAM_PRIVATE_DATABASE_HOME" \
    "args=$*"
EOF
chmod 755 "$fixture/app/usr/bin/digikam"

run_probe()
{
    env -i \
        HOME="$fixture/home" \
        PATH="/usr/bin:/bin" \
        XDG_CONFIG_HOME="$fixture/stock-config" \
        XDG_DATA_HOME="$fixture/stock-data" \
        XDG_CACHE_HOME="$fixture/stock-cache" \
        XDG_STATE_HOME="$fixture/stock-state" \
        "$@"
}

default_output=$(run_probe "$fixture/app/AppRun" --profile-probe)

grep -Fx "config=$fixture/home/.config/digikam-private" <<< "$default_output"
grep -Fx "data=$fixture/home/.local/share/digikam-private" <<< "$default_output"
grep -Fx "cache=$fixture/home/.cache/digikam-private" <<< "$default_output"
grep -Fx "state=$fixture/home/.local/state/digikam-private" <<< "$default_output"
grep -Fx "database=$fixture/home/.local/share/digikam-private" <<< "$default_output"
grep -Fx 'args=--profile-probe' <<< "$default_output"

portable_output=$(run_probe \
    DIGIKAM_PRIVATE_PROFILE_ROOT="$fixture/portable" \
    "$fixture/app/AppRun" --portable-probe)

grep -Fx "config=$fixture/portable/config" <<< "$portable_output"
grep -Fx "data=$fixture/portable/data" <<< "$portable_output"
grep -Fx "cache=$fixture/portable/cache" <<< "$portable_output"
grep -Fx "state=$fixture/portable/state" <<< "$portable_output"
grep -Fx "database=$fixture/portable/data" <<< "$portable_output"
grep -Fx 'args=--portable-probe' <<< "$portable_output"

for directory in config data cache state
do
    test "$(stat -c '%a' "$fixture/portable/$directory")" = 700
done

echo "digiKam Private AppImage profile isolation passed"
