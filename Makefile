all: ./vm/daisy ./rt.qc ./knap_15.inst.dat

./vm/daisy ./rt.qc ./knap_15.inst.dat:
	cd compiler && make clean
	cd compiler && make
	cd vm && make clean
	cd vm && make
	./compiler/donald ./rt.qc < ./examples/rt.q
	./compiler/donald ./knapsack.qc < ./examples/knapsack.q
	cp examples/knap_15.inst.dat .
	@echo "---------------------------"
	@echo "type  make run     and pray"

run: ./vm/daisy ./rt.qc ./knap_15.inst.dat
	./vm/daisy ./knapsack.qc -args ./knap_15.inst.dat	
