# Lab02 

## General 

- Auteur: Fabien Léger
- Cours: PTR, HEIG-VD
- Date: 26.03.2026

## Questions

### gettimeofday

> Résultats gettimeofday1

```shell
 0 : 1052.576718
 1 : 1052.577076
 2 : 1052.577115
 3 : 1052.577148
 4 : 1052.577423
 5 : 1052.577477
 6 : 1052.577512
 7 : 1052.577545
 8 : 1052.577576
 9 : 1052.577608
10 : 1052.577798
11 : 1052.577842
12 : 1052.577876
13 : 1052.577908
14 : 1052.578051
15 : 1052.578351
16 : 1052.578400
17 : 1052.578437
18 : 1052.578468
19 : 1052.578500
20 : 1052.578531
21 : 1052.578562
22 : 1052.578592
23 : 1052.578771
24 : 1052.578814
25 : 1052.578847
26 : 1052.578981
27 : 1052.579023
28 : 1052.579269
29 : 1052.579416
```

On voit des différences de temps conséquentes allant jusqu'à plusieurs dizaines voir centaines de micro-secondes.

> Résultats gettimeofday2

```shell
 0 : 1105.609318
 1 : 1105.609321
 2 : 1105.609322
 3 : 1105.609324
 4 : 1105.609325
 5 : 1105.609327
 6 : 1105.609329
 7 : 1105.609330
 8 : 1105.609332
 9 : 1105.609333
10 : 1105.609335
11 : 1105.609336
12 : 1105.609338
13 : 1105.609339
14 : 1105.609341
15 : 1105.609343
16 : 1105.609344
17 : 1105.609346
18 : 1105.609347
19 : 1105.609349
20 : 1105.609350
21 : 1105.609352
22 : 1105.609353
23 : 1105.609355
24 : 1105.609357
25 : 1105.609358
26 : 1105.609360
27 : 1105.609361
28 : 1105.609363
29 : 1105.609364
```

Différence de temps nettement plus faible entre les mesures. Ici, on a au plus quelques micro-secondes. En effet, cela
est dû au fait que notre boucle ne contient plus l'impression de la mesure. Celle-ci prend du temps et donc les mesures
sont plus espacées. On voit donc que la prise du temps est fortement influencée même par l'appel à une autre fonction.

> Précision de gettimeofday()

La fonction nous retourne un `struct timeval` contenant `tv_sec` en secondes et `tv_usec` en microsecondes. On peut donc
estimer que la précision de `gettimeofday()` est à la microseconde près. Je pense qu'on peut toutefois avoir des légères
modifications s'il y a une interruption avant de prendre la mesure, mais après l'appel de la fonction ? La page `man`
n'a pas fait mention d'un tel problème donc la micro-seconde semble être la meilleure estimation.