/* ******************** */
/*
 * **** **** **** / 
 * 
 * 
 * 
 * 

 * 
 * 
 * // asdasdasda///////////////////////
 * 
 * 
 */

statclass Main{

	field hello; // **/**
	
	init {
		;
                ;
	}

	fun main(args) { // **/**
		g = 0;

		for(	i = 0;		/* any statement */
			i < num;	/* expression that should yield a boolValue*/
			i = i+1)	/* same as first */
		{
			g = g + i*i;
		}
	}
}

class foo {

}

class bar extends foo {
    field hello; //chyba v kódu, ale ne na úrovni parse
}
