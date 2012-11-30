// this file contains definition of standard runtime classes of Quack


class _NullType #flags 0x8000 {
    // empty
}

class _DataBlob #flags 0xB000 {
    // empty
}

class System #flags 0x0001 {
    field in;
    field out;
    field err;

    init() #flags 0x0001 {}
}



class File #flags 0x0002 {

    field data1 #flags 0x8000;
    field data2 #flags 0x8000;
    field data3 #flags 0x8000;

    init(fileName) {
        while(!(fileName instance of String)) {
            fileName = fileName->_stringValue();
        }
        initN(fileName);
    }

    fun initN(fileName) #flags 0x0001 {}
    fun readLine() #flags 0x0001 {}

    fun writeLine(string) {
        while(!(string instance of String)) {
            string = string->_stringValue();
        }
        writeLineN(string);
    }

    fun writeLineN(string) #flags 0x0001 {}
    fun eof() #flags 0x0001 {}

    fun close() {
        finalize();
    }

    fun finalize() #flags 0x0001 {}

}

class OutFile extends File #flags 0x0002 {
    
    //init(fileName) {}

    fun initN(fileName) #flags 0x0001 {}

    fun readLine() {
        throw new IOException("Output file does not support read operation!");
    }
}

class InFile extends File #flags 0x0002 {
    
    //init(fileName) {}

    fun initN(fileName) #flags 0x0001 {}

    fun writeLine(string) {
        throw new IOException("Input file does not support write operation!");
    }
    fun writeLineN(string) {
        throw new IOException("Input file does not support write operation!");
    }
}



class Exception {
    
    field what;         // WAT!! :-O

    init(cause) {
        this.what = cause;
    }
}

class NotFoundException extends Exception {}
class IOException extends Exception {}




class String {

    field data #flags 0x8000;
    field length;
    
    init() {
        initN("");
    }

    init(string) {
        while(!(string instance of String)) {
            string = string->_stringValue();
        }
        initN(string);
    }
    
    fun _opPlus(other) {
        while(!(other instance of String)) {
            other = other->_stringValue();
        }
        _opPlusN(other);
    }
    
    fun _opIndex(index) {
        while(!(index instance of Int)) {
            index = index->_intValue();
        }
        _opIndexN(index);
    }
    
    fun explode(delim) {
        while(!(delim instance of String)) {
            delim = delim->_stringValue();
        }
        explodeN(delim);
    }
    
    // Is it needed?
    // fun _stringValue()

    fun initN(string) #flags 0x0001 {}
    fun _opPlusN(other) #flags 0x0001 {}
    fun _opIndexN(index) #flags 0x0001 {}
    fun explodeN(delim) #flags 0x0001 {}

}


class Int {

    field data #flags 0x8000;

    init(value) {
        while(!(value instance of Int)) {
            value = value->_intValue();
        }
        _initN(value);
    }

    fun initN(value) #flags 0x0001 {}

    //fun _intValue() #flags 0x0001 {}
    fun _floatValue() #flags 0x0001 {}
    fun _stringValue() #flags 0x0001 {}
    fun _boolValue() #flags 0x0001 {}


    fun _opPlus(operand) {
        while(!(operand instance of Int)) {
            operand = operand->_intValue();
        }
        _opPlusN(operand);
    }

    fun _opMinus(operand) {
        while(!(operand instance of Int)) {
            operand = operand->_intValue();
        }
        _opMinusN(operand);
    }

    fun _opMul(operand) {
        while(!(operand instance of Int)) {
            operand = operand->_intValue();
        }
        _opMulN(operand);
    }

    fun _opDiv(operand) {
        while(!(operand instance of Int)) {
            operand = operand->_intValue();
        }
        _opDivN(operand);
    }

    fun _opMod(operand) {
        while(!(operand instance of Int)) {
            operand = operand->_intValue();
        }
        _opModN(operand);
    }

    fun _opEq(operand) {
        while(!(operand instance of Int)) {
            operand = operand->_intValue();
        }
        _opEqN(operand);
    }

    fun _opNe(operand) {
        while(!(operand instance of Int)) {
            operand = operand->_intValue();
        }
        _opNeN(operand);
    }

    fun _opGt(operand) {
        while(!(operand instance of Int)) {
            operand = operand->_intValue();
        }
        _opGtN(operand);
    }

    fun _opLt(operand) {
        while(!(operand instance of Int)) {
            operand = operand->_intValue();
        }
        _opLtN(operand);
    }

    fun _opGe(operand) {
        while(!(operand instance of Int)) {
            operand = operand->_intValue();
        }
        _opGeN(operand);
    }

    fun _opLe(operand) {
        while(!(operand instance of Int)) {
            operand = operand->_intValue();
        }
        _opLeN(operand);
    }


    fun _opPlusN(secOperand) #flags 0x0001 {}
    fun _opMinusN(secOperand) #flags 0x0001 {}
    fun _opMulN(secOperand) #flags 0x0001 {}
    fun _opDivN(secOperand) #flags 0x0001 {}
    fun _opModN(secOperand) #flags 0x0001 {}

    fun _opEqN(secOperand) #flags 0x0001 {}
    fun _opNeN(secOperand) #flags 0x0001 {}
    fun _opGtN(secOperand) #flags 0x0001 {}
    fun _opLtN(secOperand) #flags 0x0001 {}
    fun _opGeN(secOperand) #flags 0x0001 {}
    fun _opLeN(secOperand) #flags 0x0001 {}

}


class Bool {

    field data #flags 0x8000;

    init(value) {
        while(!(value instance of Bool)) {
            value = value->_boolValue();
        }
        _initN(value);
    }

    fun initN(value) #flags 0x0001 {}


    fun _opEq(operand) {
        while(!(operand instance of Bool)) {
            operand = operand->_boolValue();
        }
        _opEqN(operand);
    }

    fun _opNe(operand) {
        while(!(operand instance of Bool)) {
            operand = operand->_boolValue();
        }
        _opNeN(operand);
    }

    fun _opAnd(operand) {
        while(!(operand instance of Bool)) {
            operand = operand->_boolValue();
        }
        _opAndN(operand);
    }

    fun _opOr(operand) {
        while(!(operand instance of Bool)) {
            operand = operand->_boolValue();
        }
        _opOrN(operand);
    }


    fun _opEqN(secOperand) #flags 0x0001 {}
    fun _opNeN(secOperand) #flags 0x0001 {}
    
    fun _opAndN(secOperand) #flags 0x0001 {}
    fun _opOrN(secOperand) #flags 0x0001 {}

    fun _opNeg() #flags 0x0001 {}

}


class Array {

    field data #flags 0x8000;
    field length;

    init(size) {
        while(!(size instance of Int)) {
            size = size->_intValue();
        }
        initN(size);
    }

    fun _opIndex(index) {
        while(!(index instance of Int)) {
            index = index->_intValue();
        }
        _opIndexN(index);
    }
    
    fun initN(size) #flags 0x0001 {}
    fun _opIndexN(index) #flags 0x0001 {}

}
