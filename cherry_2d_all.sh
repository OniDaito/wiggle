#!/bin/sh
touch dataset_1.log
touch dataset_2.log
touch dataset_3.log
touch dataset_4.log

./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry/20170724-QL604_S1-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170724-QL604_S1-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -o /phd/wormz/queelim/dataset_2d_all-n 100 >> dataset_2.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry/20170724-QL922_S1-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170724-QL922_S1-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -o /phd/wormz/queelim/dataset_2d_all-n 200 >> dataset_3.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry/20170724-QL923_S1-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170724-QL923_S1-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -o /phd/wormz/queelim/dataset_2d_all-n 300 >> dataset_4.log

./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry/20170724-QL925_S1-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170724-QL925_S1-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -o /phd/wormz/queelim/dataset_2d_all-n 400 >> dataset_1.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry/20170803-QL285_SB1-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170803-QL285_SB1-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -o /phd/wormz/queelim/dataset_2d_all-n 500 >> dataset_2.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry/20170803-QL285_SB2-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170803-QL285_SB2-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -o /phd/wormz/queelim/dataset_2d_all-n 600 >> dataset_3.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry/20170803-QL604_SB2-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170803-QL604_SB2-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -o /phd/wormz/queelim/dataset_2d_all-n 700 >> dataset_4.log

./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry/20170803-QL922_SB2-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170803-QL922_SB2-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -o  -n 800 >> dataset_1.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry/20170803-QL923_SB2-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation//20170803-QL923_SB2-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -o /phd/wormz/queelim/dataset_2d_all-n 900 >> dataset_2.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry/20170803-QL925_SB2-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170803-QL925_SB2-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -o /phd/wormz/queelim/dataset_2d_all-n 1000 >> dataset_3.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry/20170804-QL285_SB3-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL285_SB3-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -o /phd/wormz/queelim/dataset_2d_all-n 1100 >> dataset_4.log

./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry/20170804-QL604_SB3-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL604_SB3-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -o /phd/wormz/queelim/dataset_2d_all-n 1200 >> dataset_1.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry/20170804-QL922_SB3-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL922_SB3-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -o /phd/wormz/queelim/dataset_2d_all-n 1300 >> dataset_2.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry/20170804-QL923_SB3-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL923_SB3-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -o /phd/wormz/queelim/dataset_2d_all-n 1400 >> dataset_3.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry/20170804-QL925_SB3-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL925_SB3-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -o /phd/wormz/queelim/dataset_2d_all-n 1500 >> dataset_4.log

./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170810-QL285-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170810-QL285-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 1600 >> dataset_1.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170810-QL603-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170810-QL603-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 1700 >> dataset_2.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170810-QL806-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170810-QL806-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 1800 >> dataset_3.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170810-QL867-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170810-QL867-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 1900 >> dataset_4.log

./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170817-QL285-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170817-QL285-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 2000 >> dataset_1.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170817-QL603-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170817-QL603-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 2100 >> dataset_2.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170817-QL806-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170817-QL806-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 2200 >> dataset_3.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170817-QL867-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170817-QL867-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 2300 >> dataset_4.log

./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170818-QL285-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170818-QL285-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 2400 >> dataset_1.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170818-QL603-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170818-QL603-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 2500 >> dataset_2.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170818-QL806-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170818-QL806-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 2600 >> dataset_3.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170818-QL867-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170818-QL867-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 2700 >> dataset_4.log

./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170821-QL285-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170821-QL285-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 2800 >> dataset_1.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170821-QL569-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170821-QL569-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 2900 >> dataset_2.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170825-QL285-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170825-QL285-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 3000 >> dataset_3.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170825-QL569-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170825-QL569-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 3100 >> dataset_4.log

./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170825-QL849-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170825-QL849-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 3200 >> dataset_1.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170828-QL285-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170828-QL285-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 3300 >> dataset_2.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170828-QL569-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170828-QL569-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 3400 >> dataset_3.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170828-QL849-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170828-QL849-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 3500 >> dataset_4.log

./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170907-QL285-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170907-QL285-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 3600 >> dataset_1.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170907-QL568-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170907-QL568-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 3700 >> dataset_2.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170907-QL823-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170907-QL823-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 3800 >> dataset_3.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170907-QL824-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170907-QL824-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 3900 >> dataset_4.log

./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170907-QL835-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170907-QL835-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 4000 >> dataset_1.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170908-QL285-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170908-QL285-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 4100 >> dataset_2.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170908-QL568-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170908-QL568-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 4200 >> dataset_3.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170908-QL823-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170908-QL823-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 4300 >> dataset_4.log

./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170908-QL824-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170908-QL824-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 4400 >> dataset_1.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170908-QL835-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170908-QL835-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 4500 >> dataset_2.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170911-QL285-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170911-QL285-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 4600 >> dataset_3.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170911-QL568-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170911-QL568-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 4700 >> dataset_4.log

./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170911-QL823-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170911-QL823-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 4800 >> dataset_1.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170911-QL824-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170911-QL824-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 4900 >> dataset_2.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170911-QL835-d1.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170911-QL835-d1.0 -o /phd/wormz/queelim/dataset_2d_all-n 5000 >> dataset_3.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180126-QL285-d0.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180126-QL285-d0.0 -o /phd/wormz/queelim/dataset_2d_all-n 5100 >> dataset_4.log

./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180126-QL569-d0.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180126-QL569-d0.0 -o /phd/wormz/queelim/dataset_2d_all-n 5200 >> dataset_1.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180126-QL849-d0.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180126-QL849-d0.0 -o /phd/wormz/queelim/dataset_2d_all-n 5300 >> dataset_2.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180201-QL285-d0.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180201-QL285-d0.0 -o /phd/wormz/queelim/dataset_2d_all-n 5400 >> dataset_3.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180201-QL569-d0.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180201-QL569-d0.0 -o /phd/wormz/queelim/dataset_2d_all-n 5500 >> dataset_4.log

./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180201-QL849-d0.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180201-QL849-d0.0 -o /phd/wormz/queelim/dataset_2d_all-n 5600 >> dataset_1.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180202-QL285-d0.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180202-QL285-d0.0 -o /phd/wormz/queelim/dataset_2d_all-n 5700 >> dataset_2.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180202-QL417-d0.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180202-QL417-d0.0 -o /phd/wormz/queelim/dataset_2d_all-n 5800 >> dataset_3.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180202-QL787-d0.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180202-QL787-d0.0 -o /phd/wormz/queelim/dataset_2d_all-n 5900 >> dataset_4.log

./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180202-QL795-d0.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180202-QL795-d0.0 -o /phd/wormz/queelim/dataset_2d_all-n 6000 >> dataset_1.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180208-QL285-d0.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180208-QL285-d0.0 -o /phd/wormz/queelim/dataset_2d_all-n 6100 >> dataset_2.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180302-QL285-d0.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180302-QL285-d0.0 -o /phd/wormz/queelim/dataset_2d_all-n 6200 >> dataset_3.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180302-QL285-d0.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180302-QL285-d0.0 -o /phd/wormz/queelim/dataset_2d_all-n 6300 >> dataset_4.log

./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180302-QL849-d0.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180302-QL849-d0.0 -o /phd/wormz/queelim/dataset_2d_all-n 6400 >> dataset_1.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180308-QL285-d0.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180308-QL285-d0.0 -o /phd/wormz/queelim/dataset_2d_all-n 6500 >> dataset_2.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180308-QL569-d0.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180308-QL569-d0.0 -o /phd/wormz/queelim/dataset_2d_all-n 6600 >> dataset_3.log &
./release/graph -t -r -f -d -j 200 -w 200 -h 200 -q 10 -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180308-QL849-d0.0 -l /phd/wormz/queelim/dataset_2d_all/log.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180308-QL849-d0.0 -o /phd/wormz/queelim/dataset_2d_all-n 6700 >> dataset_4.log
