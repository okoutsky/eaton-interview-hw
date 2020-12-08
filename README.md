# Eaton Interview Homework

## Description
The assignment of the homework was following without further specification:
```
The problem to solve is following:

                You monitor devices, which are sending data to you.

                Each device have a unique name.

                Each device produces measurements.



Challange is:

                Compute number of messages you got or read from the devices.
```




The solution can be in any language (preferably C/C++).

The scope is open, you must decide how the “devices” will work in your system.

The solution should be posted on GitHub or a similar page for a review.

Please add Makefile and documentation explaining us how to run your code.


Popis reseni:
V systemu figuruji tyto komponenty:
1. zarizeni, implementovno testovaci zarizeni ctouci ze souboru - v tride file_reading device
2. klient zajistujici prenos zprav po tcp - implementace je nezavisla na konkretni serializaci, v odevzdanem reseni se vyuziva serializace do jsonu
3. server poslouchajici spojeni od klientu a parsujici jejich zpravy z prenosoveho formatu zpet do zprav.
4. storage zprav poskytujici metody pro zpetne cteni a analyzovani prijatych zprav.

komunikace mezi zarizenimi a monitorovacim serverem je implementovana pouze jednostranne. Probiha tak, ze se klient pripoji k serveru a odesila zpravy po tcp.

Pouzite knihovny:
- boost::asio pro sitovou komunikaci a casovac v file_reading_device
- nlohmann::json pro serializaci a deserializaci do jsonu
- Catch2 pro testy
