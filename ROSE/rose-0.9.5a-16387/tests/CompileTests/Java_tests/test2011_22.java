public final class test2011_22 // BadInstanceOf 
   {
     private static class Animal {}
     private static final class Fish extends Animal { void swim(){}  }
     private static final class Spider extends Animal { void crawl(){} }

     public static void doSomething(Animal aAnimal)
        {
          if (aAnimal instanceof Fish)
             {
            // Initialization is a problem
               Fish fish;
            // Fish fish = (Fish)aAnimal;
             }
        }
   }



