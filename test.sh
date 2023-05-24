passed=0
failed=0

test_diff() { 
    # $1 = File name
    f_exp="tests_expected/$1.exp"
    f_act="tests_actual/$1.act"
    
    if diff $f_exp $f_act >/dev/null; then
        echo "$1 passed"
        passed=$((passed + 1))
    else
        echo "$1 FAILED"

        expected=$(cat $f_exp)
        actual=$(cat $f_act)

        echo ""
        echo -e "EXPECTED: $expected\n"
        echo -e "ACTUAL: $actual\n"

        failed=$((failed + 1))
    fi
}
# Run tests
mkdir tests_actual

make tests >/dev/null
./tests
sleep 1

# Compare output
echo -e "TESTCASES"

# init
test_diff init
echo -e ""

# malloc
test_diff malloc1
test_diff malloc2
test_diff malloc3
test_diff malloc4
test_diff malloc5
echo -e ""

# free
test_diff free1
test_diff free2
test_diff free3
test_diff free4
test_diff free5
echo -e ""

# realloc
test_diff realloc1
test_diff realloc2
test_diff realloc3
test_diff realloc4
test_diff realloc5
test_diff realloc6
test_diff realloc_size0
test_diff realloc_ptr_null
echo -e ""

# Negative test cases
test_diff malloc_size0
test_diff malloc_fail
echo -e ""
test_diff free_ptr_null
test_diff free_fail
echo -e ""
test_diff realloc_fail
echo -e ""

total=$((passed + failed))
echo -e "$passed/$total testcases passed"

rm -r tests_actual/*
rm -d tests_actual