// this file contains definition of standard runtime classes of Quack


class _Null #flags 0x8000 {
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
        while(!(fileName instanceof String)) {
            fileName = fileName->_stringValue();
        }
        initN(fileName);
    }

    fun initN(fileName) #flags 0x0001 {}
    fun readLine() #flags 0x0001 {}

    fun writeLine(string) {
        while(!(string instanceof String)) {
            string = string->_stringValue();
        }
        return writeLineN(string);
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
        while(!(string instanceof String)) {
            string = string->_stringValue();
        }
        initN(string);
    }
    
    fun _opPlus(other) {
        while(!(other instanceof String)) {
            other = other->_stringValue();
        }
        return _opPlusN(other);
    }
    
    fun _opIndex(index) {
        while(!(index instanceof Integer)) {
            index = index->_intValue();
        }
        return _opIndexN(index);
    }
    
    fun explode(delim) {
        while(!(delim instanceof String)) {
            delim = delim->_stringValue();
        }
        return explodeN(delim);
    }
    
    // Is it needed?
    // fun _stringValue()

    fun initN(string) #flags 0x0001 {}
    fun _opPlusN(other) #flags 0x0001 {}
    fun _opIndexN(index) #flags 0x0001 {}
    fun explodeN(delim) #flags 0x0001 {}

}


class Integer {

    field data #flags 0x8000;

    init(value) {
        while(!(value instanceof Integer)) {
            value = value->_intValue();
        }
        initN(value);
    }

    fun initN(value) #flags 0x0001 {}

    //fun _intValue() #flags 0x0001 {}
    fun _floatValue() #flags 0x0001 {}
    fun _stringValue() #flags 0x0001 {}
    fun _boolValue() #flags 0x0001 {}


    fun _opPlus(operand) {
        while(!(operand instanceof Integer)) {
            operand = operand->_intValue();
        }
        return _opPlusN(operand);
    }

    fun _opMinus(operand) {
        while(!(operand instanceof Integer)) {
            operand = operand->_intValue();
        }
        return _opMinusN(operand);
    }

    fun _opMul(operand) {
        while(!(operand instanceof Integer)) {
            operand = operand->_intValue();
        }
        return _opMulN(operand);
    }

    fun _opDiv(operand) {
        while(!(operand instanceof Integer)) {
            operand = operand->_intValue();
        }
        return _opDivN(operand);
    }

    fun _opMod(operand) {
        while(!(operand instanceof Integer)) {
            operand = operand->_intValue();
        }
        return _opModN(operand);
    }

    fun _opEq(operand) {
        while(!(operand instanceof Integer)) {
            operand = operand->_intValue();
        }
        return _opEqN(operand);
    }

    fun _opNeq(operand) {
        while(!(operand instanceof Integer)) {
            operand = operand->_intValue();
        }
        return _opNeqN(operand);
    }

    fun _opGt(operand) {
        while(!(operand instanceof Integer)) {
            operand = operand->_intValue();
        }
        return _opGtN(operand);
    }

    fun _opLt(operand) {
        while(!(operand instanceof Integer)) {
            operand = operand->_intValue();
        }
        return _opLtN(operand);
    }

    fun _opGe(operand) {
        while(!(operand instanceof Integer)) {
            operand = operand->_intValue();
        }
        return _opGeN(operand);
    }

    fun _opLe(operand) {
        while(!(operand instanceof Integer)) {
            operand = operand->_intValue();
        }
        return _opLeN(operand);
    }


    fun _opPlusN(secOperand) #flags 0x0001 {}
    fun _opMinusN(secOperand) #flags 0x0001 {}
    fun _opMulN(secOperand) #flags 0x0001 {}
    fun _opDivN(secOperand) #flags 0x0001 {}
    fun _opModN(secOperand) #flags 0x0001 {}

    fun _opUnMinus() #flags 0x0001 {}

    fun _opEqN(secOperand) #flags 0x0001 {}
    fun _opNeqN(secOperand) #flags 0x0001 {}
    fun _opGtN(secOperand) #flags 0x0001 {}
    fun _opLtN(secOperand) #flags 0x0001 {}
    fun _opGeN(secOperand) #flags 0x0001 {}
    fun _opLeN(secOperand) #flags 0x0001 {}

}


class Bool {

    field data #flags 0x8000;

    init(value) {
        while(!(value instanceof Bool)) {
            value = value->_boolValue();
        }
        initN(value);
    }

    fun initN(value) #flags 0x0001 {}


    fun _opEq(operand) {
        while(!(operand instanceof Bool)) {
            operand = operand->_boolValue();
        }
        return _opEqN(operand);
    }

    fun _opNeq(operand) {
        while(!(operand instanceof Bool)) {
            operand = operand->_boolValue();
        }
        return _opNeqN(operand);
    }

    fun _opLAnd(operand) {
        while(!(operand instanceof Bool)) {
            operand = operand->_boolValue();
        }
        return _opLAndN(operand);
    }

    fun _opLOr(operand) {
        while(!(operand instanceof Bool)) {
            operand = operand->_boolValue();
        }
        return _opLOrN(operand);
    }


    fun _opEqN(secOperand) #flags 0x0001 {}
    fun _opNeqN(secOperand) #flags 0x0001 {}
    
    fun _opLAndN(secOperand) #flags 0x0001 {}
    fun _opLOrN(secOperand) #flags 0x0001 {}

    fun _opLNot() #flags 0x0001 {}

}


class Array {

    field data #flags 0x8000;
    field length;

    init(size) {
        while(!(size instanceof Integer)) {
            size = size->_intValue();
        }
        initN(size);
    }

    fun _opIndex(index) {
        while(!(index instanceof Integer)) {
            index = index->_intValue();
        }
        return _opIndexN(index);
    }
    
    fun initN(size) #flags 0x0001 {}
    fun _opIndexN(index) #flags 0x0001 {}

}
