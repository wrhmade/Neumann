#!/bin/bash
rm -rf bootcd.iso
mkisofs -graft-points -joliet -eltorito-boot bootfd.img -volid CD -path-list ./pathlist.txt -output ./bootcd.iso
