#!/bin/bash

RUN=false
DEBUG=false
OUTPUT="output.pda"
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
    *)    # unknown option
    POSITIONAL+=("$1") # save it in an array for later
    shift # past argument
    ;;
esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

cd ../bf_parser/;
make all;
cp test.e ../shared_files/bf_to_tm.e;

cd ../tm_parser/;
make all;
cp test.e ../shared_files/tm_to_pda.e;

cd ../2stackPDA_parser/;
make all;
cp test.e ../shared_files/pda_to_cm4.e;

cd ../mm4_parser/;
make all;
cp test.e ../shared_files/cm4_to_cm2.e;

cd ../shared_files/;

./bf_to_tm.e -turing $FILE > tmp.code;
./tm_to_pda.e -2StackPDA tmp.code > tmp2.code
./pda_to_cm4.e -translateTo $OUTPUT tmp2.code

if [ $RUN = true ]; then
  echo "> Running the translated code";
  if [ $DEBUG = true ]; then
    ./cm4_to_cm2.e --debug $OUTPUT;
  else
    ./cm4_to_cm2.e $OUTPUT;
  fi
fi

rm bf_to_tm.e tm_to_pda.e pda_to_cm4.e cm4_to_cm2.e tmp.code tmp2.code
