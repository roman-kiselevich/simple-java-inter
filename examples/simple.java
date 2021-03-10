class Main {
   static void run() {
      int a;
      ifj16.print("Write some number to calculate factorial: ");
      a = ifj16.readInt();
      int result;
      if (a < 0) {
         ifj16.print("Cannot calculate because input was 0");
      } else {
         result = 1;
         while (a > 0) {
            result = result * a;
            a = a - 1;
         }
         ifj16.print("Result is: " + result + "\n");
      }
   }
} // end of Main