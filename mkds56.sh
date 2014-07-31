#!/bin/bash
if [ $# -eq 0 ] ; then
	make rebuild_img PRODUCT_SERIES=ds5600
else
	make $1 PRODUCT_SERIES=ds5600
fi

