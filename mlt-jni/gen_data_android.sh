#!/user/bin/env bash

checkentry()
{
    local entry=$1
    local sub=""
    local copypath=""
    
    if [ -d $entry ];then
        for sub in $(ls $entry)
        do
            prob_sub=${entry}/$sub
            copypath=${OUTPUT_DIR}"/"$entry
            mkdir -p $copypath 
            checkentry $prob_sub
        done
    elif [ -f $entry ]; then
        entrymd5=`md5sum $entry|awk '{print $1}'`
        echo "$entry=$entrymd5"  >> $MD5_STRING_RESOURCE
        copypath=${OUTPUT_DIR}"/"$entry
        cp $entry $copypath
    fi
}

CUR_DIR=`pwd`
OUTPUT_DIR=$CUR_DIR/assets/
MD5_STRING_RESOURCE=$CUR_DIR/assets/mlt/md5check.props

mkdir -p $OUTPUT_DIR/mlt
rm $MD5_STRING_RESOURCE

cd ../mlt-standalone/output/armeabi/build/share/
DATA_DIR=`pwd`

checkentry mlt


