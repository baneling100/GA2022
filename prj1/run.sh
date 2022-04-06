
# Usage : bash run.sh n
# n should be 8 or 32 or 80
echo ""
echo "copy file from input/sample_n$1.txt to rr.in"
cp input/sample_n$1.txt rr.in
echo ""
echo "make all"
make all

echo ""
echo "make run"
timeout 180 make run 2>&1

echo ""
echo " * 'make clean' before submit or re-run"
echo " * Remove irrelevant print functions before submit."
echo " * Your program must end before time limit."
echo ""
