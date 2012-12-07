

class Item {
    field weight;
    field cost;

    init() {}
}

class InstanceFile {

    field in;
    field out;
        
    init(filename) {
        
        in = new InFile(filename);
        out = new OutFile(filename->_opPlus(".result"));

    }

    fun computeCost(conf, n, itemsArr) {
        cost = 0;

        for(i = 0; i < n; i = i + 1) {
            if((conf % 2) == 1) {
                cost = cost + itemsArr[i].cost;
            }
            conf = conf / 2;
        }

        return cost;
    }

    fun computeWeight(conf, n, itemsArr) {
        weight = 0;

        for(i = 0; i < n; i = i + 1) {
            if((conf % 2) == 1) {
                weight = weight + itemsArr[i].weight;
            }
            conf = conf / 2;
        }

        return weight;
    }

    fun bruteForce(n, M, itemsArr) {

        currentMaxCost = 0;

        currentConfig = 0;
        bestConfig = 0;
        mask = 1;

        for(i = 0; i < n; i = i + 1) {
            mask = 2 * mask;
        }
        
        while(currentConfig < mask) {

            configWeight = computeWeight(currentConfig->_intValue(), n, itemsArr);
            configCost = computeCost(currentConfig->_intValue(), n, itemsArr);
            
            if(configWeight <= M && configCost > currentMaxCost) {
                bestConfig = currentConfig;
                currentMaxCost = configCost;
            }

            currentConfig = currentConfig + 1;

        }

    }


    fun computeSolution() {

        cout = @System.out;

        inputLine = in->readLine();
        tokens = inputLine->explode(" ");

        id = tokens[0]->_intValue();
        n = tokens[1]->_intValue();
        M = tokens[2]->_intValue();

        items = new Array(n);
        
        while(!in->eof()) {

            //cout->write("Instance ID " + id + " - n = " + n + "; M = " + M);
            
            for(i = 0; i < n; i = i + 1) {
                newItem = new Item();

                newItem.weight = tokens[3 + (2*i)]->_intValue();
                newItem.cost = tokens[3 + (2*i + 1)]->_intValue();

                // ???
                items[i] = newItem;
            }
            
            cout->write("Instance successfuly loaded!");
            this->bruteForce(n, M, items);


            //////////////////////////////////////

            inputLine = in->readLine();
            tokens = inputLine->explode(" ");

            id = tokens[0]->_intValue();
            n = tokens[1]->_intValue();
            M = tokens[2]->_intValue();

        }

        in.close();
        out.close();
    
    }
}

statclass Main {

    fun main(args) {

        in = new InstanceFile("./knap_15.inst.dat");
        in->computeSolution();

    }
}