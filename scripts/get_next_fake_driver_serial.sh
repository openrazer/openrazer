#!/bin/bash

# find and sort all serials in fake driver files
serials=$(grep -h -r "r,device_serial" pylib/openrazer/_fake_driver/ | cut -d',' -f3 | sort)

# get the first serial as number (cut off the XX prefix and all leading zeroes)
first_serial_num=$(echo "$serials" | head -n1 | sed 's/XX//' | sed -e 's/^[0]*//')
# get the last serial as number (same procedure)
last_serial_num=$(echo "$serials" | tail -n1 | sed 's/XX//' | sed -e 's/^[0]*//')

# iterate through all serials between the first and the last to find holes
for i in $(seq $first_serial_num $last_serial_num); do
    # check if there is no serial matching 0$i$ -> 01$ -> 01 at the end of a line
    if [ -z $(echo "$serials" | grep "0$i$") ]; then
        # print this missing serial in the right format
        printf 'XX%010d\n' $i
        # exit as we're done
        exit 0
    fi
done

# no holes, use the next one -> last + 1
printf 'XX%010d\n' $(($last_serial_num + 1))
