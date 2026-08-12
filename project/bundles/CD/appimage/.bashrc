# .bashrc

# User specific aliases and functions

# Source global definitions
if [ -f /etc/bashrc ]; then
    . /etc/bashrc
fi

alias ll='ls -al'
alias d='digikam'
alias s='showfoto'
alias n='ninja -j4'
alias m='make -j4'
alias f='sudo make install/fast'
alias v='valgrind --tool=memcheck --leak-check=full --error-limit=no '
alias g='gdb --ex=r --args '
alias i='krazy2 --check i18ncheckarg '
alias j='krazy2 --check i18ncheckarg $(find * . 2> /dev/null)'
alias c='cppcheck -j2 -DQ_OS_LINUX --verbose --inline-suppr --platform=unix64 --enable=all --report-progress \
         --suppress=variableScope \
         --suppress=purgedConfiguration \
         --suppress=toomanyconfigs \
         --suppress=unreadVariable \
         --suppress=unusedVariable \
         --suppress=unusedStructMember \
         --suppress=unknownMacro \
         --suppress=class_X_Y \
         --suppress=ConfigurationNotChecked \
         --suppress=unmatchedSuppression'

function t() { tar -cjvf $1.tar.bz2 $1; }

# Check OS name and version.

OS_NAME=$(awk '/DISTRIB_ID=/' /etc/*-release | sed 's/DISTRIB_ID=//' | sed 's/\"//' | sed 's/\"//' | tr '[:upper:]' '[:lower:]')

if [[ $OS_NAME == "" ]] ; then
    OS_NAME=$(lsb_release -a | grep 'Distributor ID:' | sed 's/Distributor ID://' | xargs | tr '[:upper:]' '[:lower:]')
fi

OS_ARCH=$(uname -m | sed 's/x86_//;s/i[3-6]86/32/')

OS_VERSION=$(awk '/DISTRIB_RELEASE=/' /etc/*-release | sed 's/DISTRIB_RELEASE=//' | sed 's/[.]0/./')

if [[ $OS_VERSION == "" ]] ; then
    OS_VERSION=$(lsb_release -a | grep 'Release:' | sed 's/Release://' | xargs | tr '[:upper:]' '[:lower:]')
fi

echo "$OS_NAME - $OS_ARCH - $OS_VERSION"

if   [[ "$OS_NAME" == "ubuntu" ]] ; then

    # See https://askubuntu.com/questions/1209994/ssh-key-registration-not-preventing-prompting-of-password
    #     https://stackoverflow.com/questions/3466626/how-to-permanently-add-a-private-key-with-ssh-add-on-ubuntu

    added_keys=`ssh-add -l`

    if [ ! $(echo $added_keys | grep -o -e "$HOME/.ssh/id_rsa") ] ; then
        ssh-add "$HOME/.ssh/id_rsa" &
    fi

else

    # Centos like distro

    eval `SSH_ASKPASS=/usr/libexec/openssh/ssh-askpass /usr/bin/keychain -q --eval --noask --agents ssh ~/.ssh/id_rsa`

fi

export GITSLAVE=.gitslave
export SVN_EDITOR=mcedit
export QT_LOGGING_RULES="digikam*=true"

PATH=$PATH:~/Documents/scripts:/opt/cov-analysis-linux64-2021.12.1/bin:/opt/krazy/bin
export PATH

dkfrmcode()
{
    astyle --style=allman --indent=spaces=4 --convert-tabs --indent-switches \
           --break-blocks --break-closing-brackets --pad-header                \
           --align-pointer=type --indent-col1-comments --add-brackets          \
           --min-conditional-indent=0                                          \
           `find $1 -type f -name '*.cpp'` `find $1 -type f -name '*.c'` `find $1 -type f -name '*.h'`
}

dkgpg()
{
    cat ~/.gnupg/dkorg-gpg-pwd.txt | gpg --batch --yes --passphrase-fd 0 -sabv $1
    rename asc sig $1.asc
}
