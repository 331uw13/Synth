#!/bin/bash

if cmake . ; then
	if make ; then
		echo "ok!"
	fi
fi

