# COMMIT POLICY

Everybody is welcome to committing small fixes and one-liners
without prior notification to the maintainer, provided that the
following rules are followed:

    1. Please keep your commits as small and as atomic as possible.
    2. Do not push both formatting and code changes in the same commit.
    3. Do not fix coding style and code issues in the same commit.

For larger commits, please use Gitlab functionalities or send a patch to
[bugzilla](https://bugs.kde.org/describecomponents.cgi?product=digikam).
A rule of thumb to check whether your commit is a major commit is
if it affects more than 5 lines of code.

Break down larger fixes into smaller commits. Even if you push the
commits with one "git push", git preserves your commit info.

For the todo list, see bugzilla for details which is the complete story of the project.

# CODING STYLE

See the coding style detailed on **developer documentation** [available at this url](https://files.kde.org/digikam/api/)

# DOCUMENTATION

See The API documentation generated with Doxygen. You will find all information about the dependencies,
the configuration options, the rules to compile and install, and all pointers to contribute to the project as a developer.

Install Doxygen and Graphviz, and then run make doc from build directory. You can also run doxygen command line tool
directly from this folder or consult the Mainpage.dox file.

[Online version archive](https://files.kde.org/digikam/api/)

# LICENSING

See the guideline of licenses policy:

[SPDX](https://spdx.org/licenses/)
[KDE](https://community.kde.org/Policies/Licensing_Policy)

License files are hosted in LICENSE/ directory.

Usual usages:

BSD-3       : cmake, sh, rb, py, pl.
GPL2+/LGPL2+: cpp, c, h, xsl, conf, nsh, nsl.
CC0-1       : yml.
MIT         : css, js.
