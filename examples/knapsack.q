

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
        out = new OutFile(filename + ".result");

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

        currentToken = tokens->getElem(0);
        id = currentToken->_intValue();
        currentToken = tokens->getElem(1);
        n = currentToken->_intValue();
        currentToken = tokens->getElem(2);
        M = currentToken->_intValue();

        items = new Array(n);
        
        while(!in->eof()) {

            cout->write("Instance ID " + id + " - n = " + n + "; M = " + M);
            
            for(i = 0; i < n; i = i + 1) {
                newItem = new Item();

                currentToken = tokens->getElem(3 + (2*i));
                newItem.weight = currentToken->_intValue();
                currentToken = tokens->getElem(3 + (2*i + 1));
                newItem.cost = currentToken->_intValue();

                items->setElem(i, newItem);
            }
            
            cout->write("Instance successfuly loaded!");
            this->bruteForce(n, M, items);


            //////////////////////////////////////

            inputLine = in->readLine();
            tokens = inputLine->explode(" ");

            currentToken = tokens->getElem(0);
            id = currentToken->_intValue();
            currentToken = tokens->getElem(1);
            n = currentToken->_intValue();
            currentToken = tokens->getElem(2);
            M = currentToken->_intValue();
        }

        in->close();
        out->close();
    
    }
}

statclass Main {

    fun main(args) {

        in = new InstanceFile("./knap_15.inst.dat");
        in->computeSolution();

    }
}
