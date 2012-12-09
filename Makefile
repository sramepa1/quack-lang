all:
	cd compiler && make clean
	cd compiler && make
	cd vm && make clean
	cd vm && make
	./compiler/donald rt.qc < ./examples/rt.q
	./compiler/donald knapsack.qc < ./examples/knapsack.q
	cp examples/knap_15.inst.dat .
	echo "-------------------------"
	echo "ready to run"

run: ./vm/daisy ./rt.qc ./knap_15.inst.dat
	./vm/daisy ./knapsack.qc -args ./knap_15.inst.dat	
