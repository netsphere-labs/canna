#!/bin/sh
# $Id: checkout.sh,v 1.2 2002/12/21 22:44:56 aida_s Exp $
cvscmd="cvs -z 6 -d :pserver:anonymous@cvs.m17n.org:/cvs/freewnn"
$cvscmd checkout -kv -N -d cvstmp FreeWnn/Wnn/etc/xutoj.c FreeWnn/Wnn/include/ FreeWnn/Wnn/uum/ &&
$cvscmd checkout -kv -N -d cvstmp -l FreeWnn
rm -r cvstmp/CVS
