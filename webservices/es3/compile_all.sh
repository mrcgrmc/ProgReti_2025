gcc network.c serverHTTP-REST.c -o serverHTTP -lpthread
gcc network.c clientREST-GET.c -o clientREST-GET -lpthread
gcc network.c clientREST-POST.c -o clientREST-POST -lpthread
javac ClientREST.java
