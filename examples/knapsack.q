

class Item {
    field weight;
    field cost;

    init() {}
}

class InstanceFile {

    field in;
    field out;
        
    init(filename) {
        
        this.in = new InFile(filename);
        this.out = new OutFile(filename + ".result");

    }

    fun computeCost(conf, n, itemsArr) {
        cost = 0;

        for(i = 0; i < n; i = i + 1) {
            if((conf % 2) == 1) {
                currentItem = itemsArr->getElem(i);
                cost = cost + currentItem.cost;
            }
            conf = conf / 2;
        }

        return cost;
    }

    fun computeWeight(conf, n, itemsArr) {
        weight = 0;

        for(i = 0; i < n; i = i + 1) {
            if((conf % 2) == 1) {
                currentItem = itemsArr->getElem(i);
                weight = weight + currentItem.weight;
            }
            conf = conf / 2;
        }

        return weight;
    }

    fun bruteForce(n, M, itemsArr) {

        cout = @System.out;

        currentMaxCost = 0;

        currentConfig = 0;
        bestConfig = 0;
        mask = 1;

        for(i = 0; i < n; i = i + 1) {
            mask = 2 * mask;
        }
        
        while(currentConfig < mask) {

            configWeight = computeWeight(new Integer(currentConfig), n, itemsArr);
            configCost = computeCost(new Integer(currentConfig), n, itemsArr);
            
            if(configWeight <= M && configCost > currentMaxCost) {
                bestConfig = currentConfig;
                currentMaxCost = configCost;
            }

            currentConfig = currentConfig + 1;

        }

        cout->writeLine("The best cost is " + currentMaxCost);
    }


    fun computeSolution() {

        cout = @System.out;
        inField = this.in;
        outField = this.out;

        id = inField->readInt();
        n = inField->readInt();
        M = inField->readInt();

                
        items = new Array(n);
        //cout->writeLine("Instance ID " + id + " - n = " + n + "; M = " + M);

        while(!(inField->eof())) {
            
            for(i = 0; i < n; i = i + 1) {
                newItem = new Item();

                newItem.weight = inField->readInt();
                newItem.cost = inField->readInt();
                
                items->setElem(i, newItem);
            }
            
            cout->writeLine("Instance successfuly loaded!");
            this->bruteForce(n, M, items);

            //////////////////////////////////////

            id = inField->readInt();
            n = inField->readInt();
            M = inField->readInt();
        
        }

        inField->close();
        outField->close();
    
    }
}

statclass Main {

    fun main(args) {
        
        cout = @System.out;

        try {
            in = new InstanceFile("./knap_10.inst.dat");
            in->computeSolution();
        } catch e as IOException {
            cout->writeLine("Input file cannot be opened!");
            cout->writeLine("Reason: " + e.what);
        }
    }
}
