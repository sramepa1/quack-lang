all: ./vm/daisy ./rt.qc ./knap_15.inst.dat

./vm/daisy ./rt.qc ./knap_15.inst.dat:
	cd compiler && make clean
	cd compiler && make
	cd vm && make clean
	cd vm && make
	./compiler/donald ./rt.qc < ./examples/rt.q
	./compiler/donald ./knapsack.qc < ./examples/knapsack.q
	cp examples/knap_10.inst.dat .
	@echo "---------------------------"
	@echo "type  make run     and pray"
clean:
	rm -f ./vm/daisy
	rm -f ./compiler/donald
	rm -f ./rt.qc
	rm -f ./knapsack.qc
	rm -f ./knap_10.inst.dat

run: ./vm/daisy ./rt.qc ./knap_10.inst.dat
	./vm/daisy ./knapsack.qc -args ./knap_10.inst.dat	
