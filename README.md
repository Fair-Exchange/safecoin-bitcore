
=======
Zcash 1.0.5
===========

What is Zcash?
--------------

[Zcash](https://z.cash/) is an implementation of the "Zerocash" protocol.
Based on Bitcoin's code, it intends to offer a far higher standard of privacy
through a sophisticated zero-knowledge proving scheme that preserves
confidentiality of transaction metadata. Technical details are available
in our [Protocol Specification](https://github.com/zcash/zips/raw/master/protocol/protocol.pdf).

This software is the Zcash client. It downloads and stores the entire history
of Zcash transactions; depending on the speed of your computer and network
connection, the synchronization process could take a day or more once the
blockchain has reached a significant size.

Security Warnings
-----------------
 
See important security warnings in
[doc/security-warnings.md](doc/security-warnings.md).

License
-------
 
Zcash is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see http://opensource.org/licenses/MIT.
 
 
Safecoin Specific Notes
=====================
 
Dependencies
------------
 
```
#The following packages are needed:
sudo apt-get install build-essential pkg-config libc6-dev m4 g++-multilib autoconf libtool ncurses-dev unzip git python zlib1g-dev wget bsdmainutils automake libboost-all-dev libssl-dev libprotobuf-dev protobuf-compiler libqt4-dev libqrencode-dev libdb++-dev ntp ntpdate
```
 
Safecoin
------
 
```
git clone https://github.com/jl777/safecoin
cd safecoin
#./autogen.sh
#./configure --with-incompatible-bdb --with-gui
# This command might finish with: configure: error: libgmp headers missing. This can be ignored. so can libsnark directory missing error
./zcutil/fetch-params.sh

# -j8 uses 8 threads - replace 8 with number of threads you want to use
./zcutil/build.sh -j8
#This can take some time.
```
 
# to update an existing version, git checkout dPoW if not on that branch already

git pull

./zcutil/fetch-params.sh

./zcutil/build.sh -j8

To reset the blockchain, from ~/.safecoin rm -rf blocks chainstate debug.log safecoinstate db.log

Create safecoin.conf
------------------
 
```
cd ~
mkdir .safecoin
cd .safecoin
pico safecoin.conf
#Add the following lines to the safecoin.conf file:

rpcuser=bitcoinrpc
rpcpassword=password
txindex=1
addnode=5.9.102.210
addnode=78.47.196.146
addnode=178.63.69.164
addnode=88.198.65.74
addnode=5.9.122.241
addnode=144.76.94.38
addnode=89.248.166.91
```
 
Start mining
------------
 
```
#iguana documentation shows how to get the btcpubkey and wifstrs that need to be used

#bitcoin also need to be installed with txindex=1 and with rpc enabled

cd ~
cd safecoin


#This will return your pubkey eg. "0259e137e5594cf8287195d13aed816af75bd5c04ae673296b51f66e7e8346e8d8" for your address
./src/safecoin-cli validateaddress <yourwalletaddres>

#This will give the privkey of your wallet address
./src/safecoin-cli dumpprivkey <yourwalletaddres>

#This will import the privkey to be sure the mined coins are placed into your wallet address
./src/safecoin-cli importprivkey <yourwalletprivkey>

#To stop the daemon:
./src/safecoin-cli stop

#This starts safecoin notary - replace genproclimit with number of threads you want to use and add your pubkey
./src/safecoind -gen -genproclimit=2 -notary -pubkey="0259e137e5594cf8287195d13aed816af75bd5c04ae673296b51f66e7e8346e8d8" &

#This will get the stats:
./src/safecoin-cli getinfo

#To view the process:
ps -ef | grep safecoind

#To stop the daemon:
./src/safecoin-cli stop 
 
#To view safecoind output:
tail -f ~/.safecoin/debug.log

#To view all command
./src/safecoin-cli help

ASSETCHAINS: -ac_name=name -ac_supply=nnnnn

Both safecoind and safecoin-cli recognize -ac_name=option so you can create a zcash fork from the commandline

```

=======

**Zcash is unfinished and highly experimental.** Use at your own risk.

Where do I begin?
-----------------
We have a guide for joining the main Zcash network:
https://github.com/zcash/zcash/wiki/1.0-User-Guide

### Need Help?

* See the documentation at the [Zcash Wiki](https://github.com/zcash/zcash/wiki)
  for help and more information.
* Ask for help on the [Zcash](https://forum.z.cash/) forum.

Participation in the Zcash project is subject to a
[Code of Conduct](code_of_conduct.md).

Building
--------

Build Zcash along with most dependencies from source by running
./zcutil/build.sh. Currently only Linux is officially supported.

License
-------

For license information see the file [COPYING](COPYING).
