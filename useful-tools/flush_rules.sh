#! /bin/bash
ip rule flush
ip rule add from all lookup main priority 32766
ip rule add from all lookup default priority 32767
