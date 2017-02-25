#variables
divider = 1000000000000

#general plot parameters
set terminal png
set datafile separator ","

#throughput times
set title "1-Throughput per operation"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Operations per second"
set logscale y 10
set output 'lab2b_1.png'
set key right top
plot "< grep 'add-m' lab2_add.csv" using ($2):(divider/$6) \
title 'mutex add' with linespoints lc rgb 'blue', \
"< grep 'add-s' lab2_add.csv" using ($2):(divider/$6) \
title 'spinlock add' with linespoints lc rgb 'orange', \
"< grep 'list-none-m' lab2_list.csv" using ($2):(divider/$7) \
title 'mutex list' with linespoints lc rgb 'green', \
"< grep 'list-none-s' lab2_list.csv" using ($2):(divider/$7) \
title 'spinlock list' with linespoints lc rgb 'red'

#operation times
set title "2-Mutex wait times"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "mean time per operation (ns)"
set logscale y 10
set output 'lab2b_2.png'
set key left top
plot "< grep 'list-none-m' lab_2b_list.csv" using ($2):($8) \
title 'lock wait time' with linespoints lc rgb 'blue', \
"< grep 'list-none-m' lab_2b_list.csv" using ($2):($7) \
title 'avg time per operation' with linespoints lc rgb 'green'

#checking for failures
set title "3-Unprotected Threads and Iterations that run without failure"
set xlabel "Threads"
set logscale x 2
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep 'List-id-none' lab_2b_list.csv" using ($2):($3) \
	title 'yield=id' with points lc rgb 'red', \
     "< grep 'List-id-s' lab_2b_list.csv" using ($2):($3) \
	title 'yield=id w/ spin' with points pt 7 lc rgb 'purple', \
     "< grep 'List-id-m' lab_2b_list.csv" using ($2):($3) \
	title 'yield=id w/ mutex' with points lc rgb 'yellow'

#performance for mutex
set datafile separator " "
set title "4-Throughput for partitioned list with Mutex"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Operations per second"
set logscale y 10
set output 'lab2b_4.png'
set key right top
plot \
    "< grep 'List-none-m' lab_2b_list.csv | awk -F, '{if($4 == 1){print $2,(1000000000000/$7)}}'" \
    title 'lists=1' with linespoints lc rgb 'blue', \
    "< grep 'List-none-m' lab_2b_list.csv | awk -F, '{if($4 == 4){print $2,(1000000000000/$7)}}'" \
    title 'lists=4' with linespoints lc rgb 'yellow', \
    "< grep 'List-none-m' lab_2b_list.csv | awk -F, '{if($4 == 8){print $2,(1000000000000/$7)}}'" \
    title 'lists=8' with linespoints lc rgb 'red', \
    "< grep 'List-none-m' lab_2b_list.csv | awk -F, '{if($4 == 16){print $2,(1000000000000/$7)}}'" \
    title 'lists=12' with linespoints lc rgb 'green'


#performance for spinlock
set datafile separator " "
set title "5-Throughput for partitioned list with spinlock"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Operations per second"
set logscale y 10
set output 'lab2b_5.png'
set key right top
plot \
    "< grep 'List-none-s' lab_2b_list.csv | awk -F, '{if($4 == 1){print $2,(1000000000000/$7)}}'" \
    title 'lists=1' with linespoints lc rgb 'blue', \
    "< grep 'List-none-s' lab_2b_list.csv | awk -F, '{if($4 == 4){print $2,(1000000000000/$7)}}'" \
    title 'lists=4' with linespoints lc rgb 'yellow', \
    "< grep 'List-none-s' lab_2b_list.csv | awk -F, '{if($4 == 8){print $2,(1000000000000/$7)}}'" \
    title 'lists=8' with linespoints lc rgb 'red', \
    "< grep 'List-none-s' lab_2b_list.csv | awk -F, '{if($4 == 16){print $2,(1000000000000/$7)}}'" \
    title 'lists=16' with linespoints lc rgb 'green'