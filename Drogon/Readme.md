Drogon framework

--------------------------------
install on Manjaro Linux

yay -Ss drogon

yay -Syu
yay -S drogon
---------------
confirm the installation with these commands.

### 1. Check Drogon

drogon_ctl --version

You should see something like:

1.9.13

### 2. Check where it is installed

which drogon_ctl

Likely:

/usr/bin/drogon_ctl

### 3. Check the installed package

pacman -Q drogon

You should see:

drogon 1.9.13-1
-------------------------------------------------------
