#!/bin/bash
sudo cp meego-ubuntu-10.10.list /etc/apt/sources.list.d/
gpg --keyserver pgpkeys.mit.edu --recv 0BC7BEC479FC1F8A
gpg --export --armor 0BC7BEC479FC1F8A | sudo apt-key add -
sudo apt-get update
VERIFIED=`apt-cache policy madde|grep meego`
if [[ -z $VERIFIED ]]; then
  echo "Repository configuration could not be verified."
  exit 123
fi

sudo apt-get install meego-sdk --force-yes --yes
