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

## Recover a Strong Privacy original without digiKam

Strong Privacy originals live inside the category's gocryptfs store, under
plaintext names such as `originals/<container-uuid>/`. The complete ciphertext
store directory, its `gocryptfs.conf` and the category password are the normal
portable recovery unit; digiKam Private is not required.

Copy the complete store directory while digiKam is closed, then mount the copy
with the category password:

```sh
mkdir recovered-store
gocryptfs /path/to/copied/store recovered-store
```

The original and associated members appear below
`recovered-store/originals/<container-uuid>/`. Compare every restored member
against the SHA-256 hashes recorded in the P1 catalogue before replacing any
collection file, then copy the members to the restored location. Keep the
ciphertext store read-only and do not edit it directly.

Every Strong store also contains a versioned encrypted portable manifest at
`digikam-private/recovery-v1.json` inside the mounted store. It records the
category identity and settings, the store identity, and the complete
item/asset mapping: vault paths, logical public paths, original names, roles,
orders, hashes, sizes, timestamps and portable attributes. A copied complete
store plus its manifest can be discovered, authenticated and reconstructed in
a fresh digiKam Private profile without the original P1 database.

A wrong password is rejected; there is no password reset or recovery-key
dialog in normal category use. The emergency path exports the master key
contained in `gocryptfs.conf` with the category password:

```sh
gocryptfs-xray -dumpmasterkey /path/to/copied/store/gocryptfs.conf
```

The exported key can then mount the store with `gocryptfs -masterkey=stdin`
(or the equivalent recovery input form). Treat the exported key exactly like
the category password: it unlocks every original in that store. The normal
digiKam Private authentication path never dumps or parses the master key.

## Restore into digiKam

Restore while digiKam is closed. Restore the database, complete collection
roots, and complete Managed Store Root as one consistent backup set. Start
digiKam with the collection and store volumes mounted, then let its startup
recovery and integrity check finish before changing files. Preserve any files
it reports as unknown or changed until they have been reconciled.
