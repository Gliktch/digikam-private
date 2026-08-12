eval `ssh-agent`

added_keys=`ssh-add -l`
echo "Registered ssh keys: $added_keys"

if [ ! $(echo $added_keys | grep -o -e "$HOME/.ssh/id_rsa") ] ; then
    ssh-add "$HOME/.ssh/id_rsa" &
fi
