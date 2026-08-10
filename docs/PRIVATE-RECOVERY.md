# digiKam Private backup and recovery

digiKam Private is designed so that Casual Privacy originals remain recoverable
without digiKam. It does not include password guessing or a forgotten-password
workflow. Keep the category passwords with your normal password backups.

## What to back up

Back up complete collection roots while digiKam is closed. This preserves:

* visible public proxies
* adjacent `<original-name>.<ext>.digikam-private.zip` archives
* root-local `.digikam-private/transactions/` recovery journals
* any interrupted staging files

Also back up the complete configured Managed Store Root, including its
`.digikam-private/root-marker-v1.json`, `stores/`, `transactions/`, and staging
content. The store contains encrypted presentation derivatives and can contain
unfinished external-edit recovery data.

Back up the configured core database (`digikam4.db` for a typical SQLite
setup) and preferably the digiKam configuration file. Thumbnail, recognition,
and similarity databases are useful caches, but they are not authoritative
private-media recovery data.

A disconnected collection root is not included merely because digiKam records
it in the catalogue. Back up that root separately when it is mounted. Do not
back up `$XDG_RUNTIME_DIR/digikam-private/`; it is temporary plaintext runtime
state.

## Recover a Casual Privacy original without digiKam

Each adjacent `.digikam-private.zip` file is a standard ZipCrypto archive. Use
a normal ZIP tool and enter the category password when prompted. For example:

```sh
mkdir recovered
unzip holiday.jpg.digikam-private.zip -d recovered
```

Do not put the password on a shared shell command line. The extracted archive
contains:

* `digikam-private/recovery-v1.json`, describing the item and its members
* exact original and associated files below `digikam-private/assets/`

The manifest records the original names, roles, ordering, hashes, sizes, and
portable timestamps/attributes needed to identify and restore the members.
Verify hashes from the manifest before replacing collection files.

ZipCrypto is intentionally weak against specialist recovery. That supports the
project's recoverable front-door-lock goal; it is not strong encryption.

## Open an encrypted category store

The Managed Store Root uses the standard gocryptfs format. Preserve the whole
cipher directory, especially `gocryptfs.conf`, then mount a copied store into
an empty directory with the category password:

```sh
mkdir clear-store
gocryptfs /path/to/copied/cipher-store clear-store
```

For v1 Casual Privacy, these stores are convenience and transaction data; the
adjacent ZIP archives remain the authoritative original-media recovery path.
Never edit the ciphertext tree directly.

## Restore into digiKam

Restore while digiKam is closed. Restore the database, complete collection
roots, and complete Managed Store Root as one consistent backup set. Start
digiKam with the collection and store volumes mounted, then let its startup
recovery and integrity check finish before changing files. Preserve any files
it reports as unknown or changed until they have been reconciled.
