// Simplest java example (note that class name must match filename).

public class test2011_17
   {
  // This is the required "main" function.
     public static void main(String args[])
        {
          int x = 7;
          int y = 8;
          int z = 9;
          boolean boolean_x = true;
          boolean boolean_y = false;
          boolean boolean_z = true;

          x = y + z;
          x = y - z;
          x = y / z;
          x = y * z;
          x = y | z;
          boolean_x = boolean_y || boolean_z;
          x = y & z;
          boolean_x = boolean_y && boolean_z;
          x = y ^ z;
          boolean_x = y < z;
          boolean_x = y <= z;
          boolean_x = y > z;
          boolean_x = y >= z;
          boolean_x = y == z;
          x = y >> z;
          x = y << z;
          x = y % z;

       // These will be passed through as type names...all the way to the ECJ traversal
       // unsigned y_u = 8;
       // unsigned z_u = 9;
          x = y >>> z;

          boolean_x = y != z;
        }
   }
