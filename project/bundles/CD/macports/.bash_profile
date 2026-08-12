
export PATH=$PATH:/opt/kroleg/mc/bin
export TERMINFO=/usr/share/terminfo
export GITSLAVE=.gitslave.devel
export LC_CTYPE=en_US.UTF-8
export QT_LOGGING_RULES="digikam*=true"
export EDITOR="mcedit"
export VISUAL="mcedit"

alias m='make -j8'
alias ll='ls -al'
alias o='otool -L'

if [ ! -S ~/.ssh/ssh_auth_sock ]; then
  eval `ssh-agent`
  ln -sf "$SSH_AUTH_SOCK" ~/.ssh/ssh_auth_sock
fi
export SSH_AUTH_SOCK=~/.ssh/ssh_auth_sock
ssh-add -l > /dev/null || ssh-add

tree()
{
    [ -n "$2" ] && local depth="-maxdepth $2";
    find "${1:-.}" ${depth} -print 2> /dev/null | sed -e 's;[^/]*/;|____;g;s;____|; |;g'
}