#!/usr/bin/env bash
SRC=${BASH_SOURCE[0]}
[[ $SRC != /* ]] && SRC=$PWD/$SRC
source ${SRC%/*}/cg_test_inc.sh


is_near(){
    local a=$1
    local b=$2
    echo  " a: $a b: $b " >&2
    local diff=$((a-b))
    ((diff<0)) && ((diff=-diff))
    echo diff=$diff >&2
    echo $((diff<3))
}

test_cmd_output(){
    local c=$1 expected=$2 in=${3:-}
    local cmd="$c -$ $in"
    $cmd
    local out="$RETVAL"
    echo "$cmd  -->  $out"
    assert_eq "$out" "$expected"
    echo ------------------------------------------------------------
}

test_normalizePath(){
    test_cmd_output  cg_normalizePath /a/b/c/d/e  /a////b//c/./d/./././e
    local expected=/s-mcpb-ms03/store/PRO3/Maintenance/202312/2023_abc.zip z
    for z in {//s-mcpb-ms03/fularchiv01.d/EquiValent/152/1/,//s-mcpb-ms03/fularchiv01/1/,//s-mcpb-ms03/store}//PRO3/Maintenance/202312/2023_abc.zip; do
        test_cmd_output  cg_normalizePath  $expected  $z
    done
    local dir
    for dir in mysubdir ./; do
        mkdir -p $dir
        pushd $dir
        test_cmd_output cg_normalizePath  "$PWD/a/b/c/d/e"  a////b//c/./d/./././e
        popd
    done
}


main(){
    RETVAL=(a b c ) # should not interfere
    mkdir -p $DIR || return
    ls -l $DIR
    cd $DIR || return
    echo Hello > hello.txt
    local now=$(date +%s)
    local lm=$(date +%s -r hello.txt)
    test_normalizePath
    echo ------------------------------------------------------------
    local cmd='cg_fileSize hello.txt'
    local s=$(cg_fileSize hello.txt)
    echo "$cmd -> $s"
    assert_eq "$s" 6
    test_cmd_output cg_fileSize 6 hello.txt
        test_cmd_output cg_fileSize 0 nonExistingFile.txt
    test_cmd_output cg_fileSizeCommaLastModified "6,$lm" hello.txt
    echo ------------------------------------------------------------
    local cmd='cg_currentTimeSeconds -$'
    $cmd
    local s="$RETVAL"
    echo "$cmd -> $s"
    assert_eq $(is_near "$s" "$now") 1
    echo ------------------------------------------------------------
    local cmd='cg_fileUnmodifiedSeconds -$ hello.txt'
    $cmd
    local s="$RETVAL"
    echo "$cmd -> $s"
    assert_eq $(is_near "$s" 0) 1
    ((s+=10))
    assert_eq $(is_near "$s" 0) 0
    echo ------------------------------------------------------------
    local cmd='cg_areFilesUnmodifiedSeconds -$ 3 /etc/fstab /etc/os-release hello.txt'
    local res=1
    $cmd && res=0
    local s="$RETVAL"
    echo "$cmd -> $s  res: $res"
    assert_eq "$s" hello.txt
    assert_eq "$res" 1
    echo ------------------------------------------------------------
    local cmd='cg_areFilesUnmodifiedSeconds -$ 3 /etc/fstab /etc/os-release'
    local res=1
    $cmd && res=0
    local s="$RETVAL"
    echo "$cmd -> $s  res: $res"
    assert_eq "$s" ''
    assert_eq "$res" 0
    echo ------------------------------------------------------------
    local i
    for((i=0;i<3;i++)); do
        local expect_res=0
        ((i==2)) && my_local=1 && expect_res=1
        local cmd="cg_lowercase_global_variables"
        local res=0
        $cmd || res=1
        echo "$cmd   ->  res: $res"
        assert_eq $res $expect_res
        echo
    done
}

main "$@"
