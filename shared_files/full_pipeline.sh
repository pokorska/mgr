#!/bin/bash

RUN=false
TRANSLATE=true
DEBUG=false
CLEAN=false
KEEP=false
STAGES=4
TO_ARG="mm2"
CUSTOM_OUTPUT=false
OUTPUT_TMP=""

TM_OUT="output.tm"
PDA_OUT="output.pda"
MM4_OUT="output_mm4/base"
MM2_OUT="output_mm2/base"
OUTPUT=$MM2_OUT

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -f|--file)
    FILE="$2"
    shift # past argument
    shift # past value
    ;;
    -o|--output)
    CUSTOM_OUTPUT=true
    OUTPUT_TMP="$2"
    shift # past argument
    shift # past value
    ;;
    -r|--run)
    RUN=true
    shift # past argument
    ;;
    --debug)
    DEBUG=true
    shift
    ;;
    --recompile|--clean)
    CLEAN=true
    shift
    ;;
    --keep)
    KEEP=true
    shift
    ;;
    -t|--to)
    TO_ARG="$2"
    shift
    shift
    ;;
    -d|--direct)
    TRANSLATE=false
    RUN=true
    FILE="$2"
    shift
    shift
    ;;
    *)    # unknown option
    POSITIONAL+=("$1") # save it in an array for later
    shift # past argument
    ;;
esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

TO_ARG_LOWER=`echo "$TO_ARG" | tr '[:upper:]' '[:lower:]'`;

if [ $TO_ARG_LOWER = "mm2" ]; then STAGES=4
elif [ $TO_ARG_LOWER = "mm4" ]; then STAGES=3
elif [ $TO_ARG_LOWER = "pda" ] || [ $TO_ARG_LOWER == "2stackpda" ]; then STAGES=2
elif [ $TO_ARG_LOWER = "tm" ]; then STAGES=1
elif [ $TO_ARG_LOWER = "bf" ]; then STAGES=0
fi

if [ $STAGES = 0 ]; then OUTPUT=$FILE;
elif [ $STAGES = 1 ]; then OUTPUT=$TM_OUT;
elif [ $STAGES = 2 ]; then OUTPUT=$PDA_OUT;
elif [ $STAGES = 3 ]; then OUTPUT=$MM4_OUT;
elif [ $STAGES = 4 ]; then OUTPUT=$MM2_OUT;
fi

if [ $CUSTOM_OUTPUT = true ]; then OUTPUT=$OUTPUT_TMP; fi

cd ../bf_parser/;
if [ $CLEAN = true ]; then make clean; fi
make all;
cp test.e ../shared_files/bf_to_tm.e;

cd ../tm_parser/;
if [ $CLEAN = true ]; then make clean; fi
make all;
cp test.e ../shared_files/tm_to_pda.e;

cd ../2stackPDA_parser/;
if [ $CLEAN = true ]; then make clean; fi
make all;
cp test.e ../shared_files/pda_to_cm4.e;

cd ../mm4_parser/;
if [ $CLEAN = true ]; then make clean; fi
make all;
cp test.e ../shared_files/cm4_to_cm2.e;

cd ../mm2_parser/;
if [ $CLEAN = true ]; then make clean; fi
make all;
cp test.e ../shared_files/cm2.e;

cd ../shared_files/;

if [ $TRANSLATE = true ]; then
  if [ $STAGES -gt 2 ]; then
    DIRNAME=`dirname $OUTPUT`;
    if [ -d $DIRNAME ]; then
      echo "Output directory $DIRNAME already exists. I'm removing all its content.";
      #rm -Rf $DIRNAME;
    fi
    mkdir $DIRNAME;
  fi
  echo "";
  if [ $STAGES -gt 0 ]; then
    echo "Translation from Brainfuck to Turing Machine... ";
    OUT=$TM_OUT; if [ $STAGES = 1 ]; then OUT=$OUTPUT; fi
    ./bf_to_tm.e -turing $FILE > $OUT;
    echo "completed."; echo "";
  fi
  if [ $STAGES -gt 1 ]; then
    echo "Translation from Turing Machine to 2 stack PDA... ";
    OUT=$PDA_OUT; if [ $STAGES = 2 ]; then OUT=$OUTPUT; fi
    ./tm_to_pda.e -2StackPDA $TM_OUT > $OUT;
    echo "completed."; echo "";
  fi
  if [ $STAGES -gt 2 ]; then
    echo "Translation from 2 stack PDA to 4 Counter Machine... ";
    OUT=$MM4_OUT; if [ $STAGES = 3 ]; then OUT=$OUTPUT; fi
    ./pda_to_cm4.e -minsky -multifile $OUT $PDA_OUT;
    echo "completed."; echo "";
  fi
  if [ $STAGES -gt 3 ]; then
    echo "Translation from 4 Counter Machine to 2 Counter Machine...";
    ./cm4_to_cm2.e -minsky -multifile $MM4_OUT -o $OUTPUT;
    echo "completed."; echo "";
  fi
else
  OUTPUT=$FILE;
  filename=$(basename -- "$FILE")
  ext="${filename##*.}"
  if [ $ext = "bf" ]; then STAGES=0; fi
  if [ $ext = "tm" ]; then STAGES=1; fi
  if [ $ext = "pda" ]; then STAGES=2; fi
  if [ $ext = "mm4" ]; then STAGES=3; fi
  if [ $ext = "mm2" ]; then STAGES=4; fi
fi

if [ $RUN = true ]; then
  echo "";
  echo "> Running the translated code";
  if [ $STAGES = 0 ]; then
    ./bf_to_tm.e $OUTPUT;
  elif [ $STAGES = 1 ]; then
    ./tm_to_pda.e $OUTPUT;
  elif [ $STAGES = 2 ]; then
    ./pda_to_cm4.e $OUTPUT;
  elif [ $STAGES = 3 ]; then
    ./cm4_to_cm2.e -multifile $OUTPUT;
  elif [ $STAGES = 4 ]; then
    ./cm2.e -multifile $OUTPUT;
  fi
fi

CODES=($TM_OUT $PDA_OUT `dirname $MM4_OUT` `dirname $MM2_OUT`)

rm bf_to_tm.e tm_to_pda.e pda_to_cm4.e cm4_to_cm2.e cm2.e 2> /dev/null
if [ $KEEP = false ]; then
  for file in ${CODES[*]}; do
    if [ $file != $OUTPUT ] &&  [ $file != `dirname $OUTPUT` ]; then rm -Rf $file 2> /dev/null; fi
  done
fi
