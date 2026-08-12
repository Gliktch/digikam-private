Craft CI/CD Repositories
------------------------

KDE uses a common framework named Craft based on Python to process applications
continuous workflow on different platforms:

- Continuous Integration (CI - Build Factory).
- Continuous Deployment (CD - Binary Factory).

The workflow supports notarization of application bundles (signing) for Linux (AppImage and Snap), Windows, and MacOS stores.

* Documentations and tutorials:

- Front page                    : https://community.kde.org/Craft
- Settings details              : https://community.kde.org/Craft/Blueprints
- Introduction to binary factory: https://kfunk.org/2017/11/17/kde-binary-factory/

* Git repositories:

- KF dependencies used by Craft : https://invent.kde.org/sysadmin/repo-metadata/-/tree/master/dependencies
- Craft core implementation     : https://invent.kde.org/packaging/craft
- Configuration                 : https://invent.kde.org/packaging/craft-blueprints-kde/-/tree/master/extragear/digikam
- Binary factory customizations : https://invent.kde.org/sysadmin/binary-factory-tooling/-/blob/master/craft/enabled-projects.yaml

* Jenkins Web interfaces:

- Build Factory Jenkins page    : https://build.kde.org/
- Binary Factory Jenkins page   : https://binary-factory.kde.org/

Notes:

- All changes performed to Binary Factory customizations files needs a DSL Job to be triggered by KDE sysadmin team.
  This job will synchronize the settings to the Binary Factory environnments.

- Changes from Binary Factory customizations allow to specify which version of dependencies must by installed to compile digiKam.
  For each dependency versions installed, the same release must be specified in build factory configuration.

- The Craft Core python repository host all low level implementation and rules to use in Craft BluePrints configurations.
