#!/bin/sh
touch dataset_1.log
touch dataset_2.log
touch dataset_3.log
touch dataset_4.log

./release/count -b -i /phd/wormz/queelim/ins-6-mCherry/20170724-QL285_S1-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170724-QL285_S1-d1.0  -l /phd/wormz/queelim/counts.csv -o /phd/wormz/queelim/  >> dataset_1.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry/20170724-QL604_S1-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170724-QL604_S1-d1.0 -l /phd/wormz/queelim/counts.csv -o /phd/wormz/queelim/ >> dataset_2.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry/20170724-QL922_S1-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170724-QL922_S1-d1.0 -l /phd/wormz/queelim/counts.csv -o /phd/wormz/queelim/ >> dataset_3.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry/20170724-QL923_S1-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170724-QL923_S1-d1.0 -l /phd/wormz/queelim/counts.csv -o /phd/wormz/queelim/ >> dataset_4.log

./release/count -b -i /phd/wormz/queelim/ins-6-mCherry/20170724-QL925_S1-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170724-QL925_S1-d1.0 -l /phd/wormz/queelim/counts.csv -o /phd/wormz/queelim/ >> dataset_1.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry/20170803-QL285_SB1-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170803-QL285_SB1-d1.0 -l /phd/wormz/queelim/counts.csv -o /phd/wormz/queelim/  >> dataset_2.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry/20170803-QL285_SB2-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170803-QL285_SB2-d1.0 -l /phd/wormz/queelim/counts.csv -o /phd/wormz/queelim/  >> dataset_3.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry/20170803-QL604_SB2-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170803-QL604_SB2-d1.0 -l /phd/wormz/queelim/counts.csv -o /phd/wormz/queelim/  >> dataset_4.log

./release/count -b -i /phd/wormz/queelim/ins-6-mCherry/20170803-QL922_SB2-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170803-QL922_SB2-d1.0 -l /phd/wormz/queelim/counts.csv -o /phd/wormz/queelim/  >> dataset_1.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry/20170803-QL923_SB2-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation//20170803-QL923_SB2-d1.0 -l /phd/wormz/queelim/counts.csv -o /phd/wormz/queelim/ >> dataset_2.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry/20170803-QL925_SB2-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170803-QL925_SB2-d1.0 -l /phd/wormz/queelim/counts.csv -o /phd/wormz/queelim/  >> dataset_3.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry/20170804-QL285_SB3-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL285_SB3-d1.0 -l /phd/wormz/queelim/counts.csv -o /phd/wormz/queelim/  >> dataset_4.log

./release/count -b -i /phd/wormz/queelim/ins-6-mCherry/20170804-QL604_SB3-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL604_SB3-d1.0 -l /phd/wormz/queelim/counts.csv -o /phd/wormz/queelim/ >> dataset_1.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry/20170804-QL922_SB3-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL922_SB3-d1.0 -l /phd/wormz/queelim/counts.csv -o /phd/wormz/queelim/ >> dataset_2.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry/20170804-QL923_SB3-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL923_SB3-d1.0 -l /phd/wormz/queelim/counts.csv -o /phd/wormz/queelim/ >> dataset_3.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry/20170804-QL925_SB3-d1.0 -a /phd/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL925_SB3-d1.0 -l /phd/wormz/queelim/counts.csv -o /phd/wormz/queelim/ >> dataset_4.log

./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170810-QL285-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170810-QL285-d1.0 -o /phd/wormz/queelim/ >> dataset_1.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170810-QL603-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170810-QL603-d1.0 -o /phd/wormz/queelim/ >> dataset_2.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170810-QL806-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170810-QL806-d1.0 -o /phd/wormz/queelim/ >> dataset_3.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170810-QL867-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170810-QL867-d1.0 -o /phd/wormz/queelim/ >> dataset_4.log

./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170817-QL285-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170817-QL285-d1.0 -o /phd/wormz/queelim/ >> dataset_1.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170817-QL603-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170817-QL603-d1.0 -o /phd/wormz/queelim/ >> dataset_2.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170817-QL806-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170817-QL806-d1.0 -o /phd/wormz/queelim/ >> dataset_3.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170817-QL867-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170817-QL867-d1.0 -o /phd/wormz/queelim/ >> dataset_4.log

./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170818-QL285-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170818-QL285-d1.0 -o /phd/wormz/queelim/ >> dataset_1.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170818-QL603-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170818-QL603-d1.0 -o /phd/wormz/queelim/ >> dataset_2.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170818-QL806-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170818-QL806-d1.0 -o /phd/wormz/queelim/ >> dataset_3.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170818-QL867-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170818-QL867-d1.0 -o /phd/wormz/queelim/ >> dataset_4.log

./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170821-QL285-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170821-QL285-d1.0 -o /phd/wormz/queelim/ >> dataset_1.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170821-QL569-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170821-QL569-d1.0 -o /phd/wormz/queelim/ >> dataset_2.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170825-QL285-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170825-QL285-d1.0 -o /phd/wormz/queelim/ >> dataset_3.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170825-QL569-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170825-QL569-d1.0 -o /phd/wormz/queelim/ >> dataset_4.log

./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170825-QL849-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170825-QL849-d1.0 -o /phd/wormz/queelim/ >> dataset_1.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170828-QL285-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170828-QL285-d1.0 -o /phd/wormz/queelim/ >> dataset_2.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170828-QL569-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170828-QL569-d1.0 -o /phd/wormz/queelim/ >> dataset_3.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170828-QL849-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170828-QL849-d1.0 -o /phd/wormz/queelim/ >> dataset_4.log

./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170907-QL285-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170907-QL285-d1.0 -o /phd/wormz/queelim/ >> dataset_1.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170907-QL568-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170907-QL568-d1.0 -o /phd/wormz/queelim/ >> dataset_2.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170907-QL823-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170907-QL823-d1.0 -o /phd/wormz/queelim/ >> dataset_3.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170907-QL824-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170907-QL824-d1.0 -o /phd/wormz/queelim/ >> dataset_4.log

./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170907-QL835-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170907-QL835-d1.0 -o /phd/wormz/queelim/ >> dataset_1.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170908-QL285-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170908-QL285-d1.0 -o /phd/wormz/queelim/ >> dataset_2.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170908-QL568-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170908-QL568-d1.0 -o /phd/wormz/queelim/ >> dataset_3.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170908-QL823-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170908-QL823-d1.0 -o /phd/wormz/queelim/ >> dataset_4.log

./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170908-QL824-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170908-QL824-d1.0 -o /phd/wormz/queelim/ >> dataset_1.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170908-QL835-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170908-QL835-d1.0 -o /phd/wormz/queelim/ >> dataset_2.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170911-QL285-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170911-QL285-d1.0 -o /phd/wormz/queelim/ >> dataset_3.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170911-QL568-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170911-QL568-d1.0 -o /phd/wormz/queelim/ >> dataset_4.log

./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170911-QL823-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170911-QL823-d1.0 -o /phd/wormz/queelim/ >> dataset_1.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170911-QL824-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170911-QL824-d1.0 -o /phd/wormz/queelim/ >> dataset_2.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20170911-QL835-d1.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170911-QL835-d1.0 -o /phd/wormz/queelim/ >> dataset_3.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180126-QL285-d0.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180126-QL285-d0.0 -o /phd/wormz/queelim/ >> dataset_4.log

./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180126-QL569-d0.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180126-QL569-d0.0 -o /phd/wormz/queelim/ >> dataset_1.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180126-QL849-d0.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180126-QL849-d0.0 -o /phd/wormz/queelim/ >> dataset_2.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180201-QL285-d0.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180201-QL285-d0.0 -o /phd/wormz/queelim/ >> dataset_3.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180201-QL569-d0.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180201-QL569-d0.0 -o /phd/wormz/queelim/ >> dataset_4.log

./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180201-QL849-d0.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180201-QL849-d0.0 -o /phd/wormz/queelim/ >> dataset_1.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180202-QL285-d0.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180202-QL285-d0.0 -o /phd/wormz/queelim/ >> dataset_2.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180202-QL417-d0.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180202-QL417-d0.0 -o /phd/wormz/queelim/ >> dataset_3.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180202-QL787-d0.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180202-QL787-d0.0 -o /phd/wormz/queelim/ >> dataset_4.log

./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180202-QL795-d0.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180202-QL795-d0.0 -o /phd/wormz/queelim/ >> dataset_1.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180208-QL285-d0.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180208-QL285-d0.0 -o /phd/wormz/queelim/ >> dataset_2.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180302-QL285-d0.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180302-QL285-d0.0 -o /phd/wormz/queelim/ >> dataset_3.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180302-QL285-d0.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180302-QL285-d0.0 -o /phd/wormz/queelim/ >> dataset_4.log

./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180302-QL849-d0.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180302-QL849-d0.0 -o /phd/wormz/queelim/ >> dataset_1.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180308-QL285-d0.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180308-QL285-d0.0 -o /phd/wormz/queelim/ >> dataset_2.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180308-QL569-d0.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180308-QL569-d0.0 -o /phd/wormz/queelim/ >> dataset_3.log &
./release/count -b -i /phd/wormz/queelim/ins-6-mCherry_2/20180308-QL849-d0.0 -l /phd/wormz/queelim/counts.csv -a /phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180308-QL849-d0.0 -o /phd/wormz/queelim/ >> dataset_4.log
