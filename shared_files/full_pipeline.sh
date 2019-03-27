#!/bin/bash

RUN=false
TRANSLATE=true
DEBUG=false
CLEAN=false
KEEP=false
OUTPUT="output_mm4/base"
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
    OUTPUT="$2"
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
    -d|--direct)
    TRANSLATE=false
    RUN=true
    OUTPUT="$2"
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

cd ../shared_files/;

if [ $TRANSLATE = true ]; then
  DIRNAME=`dirname $OUTPUT`;
  if [ -d $DIRNAME ]; then
    echo "Output directory $DIRNAME already exists. I'm removing all its content.";
    rm -Rf $DIRNAME;
  fi
  mkdir $DIRNAME;
  echo "Translation from Brainfuck to Turing Machine... ";
  ./bf_to_tm.e -turing $FILE > tmp.tm;
  echo "completed.";
  echo "Translation from Turing Machine to 2 stack PDA... ";
  ./tm_to_pda.e -2StackPDA tmp.tm > tmp.pda;
  echo "completed.";
  echo "Translation from 2 stack PDA to 4 Counter Machine... ";
  ./pda_to_cm4.e -multifile "$OUTPUT" tmp.pda;
  echo "completed.";
fi

if [ $RUN = true ]; then
  echo "> Running the translated code";
  if [ $DEBUG = true ]; then
    ./cm4_to_cm2.e --debug -multifile "$OUTPUT";
  else
    ./cm4_to_cm2.e -multifile "$OUTPUT";
  fi
fi
rm bf_to_tm.e tm_to_pda.e pda_to_cm4.e cm4_to_cm2.e 2> /dev/null
if [ $KEEP = false ]; then
  rm tmp.tm tmp.pda 2> /dev/null
fi
