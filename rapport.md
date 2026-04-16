# Lab02 

## General 

- Auteur: Fabien Léger
- Cours: PTR, HEIG-VD
- Date: 26.03.2026

## Questions

### Développement

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

### Horloges POSIX

```shell
=== CLOCK_REALTIME ===
Theoretical resolution : 0.000000001 s
 0 : 1524.984063860
 1 : 1524.984065900
 2 : 1524.984067530
 3 : 1524.984069070
 4 : 1524.984070590
 5 : 1524.984072140
 6 : 1524.984073690
 7 : 1524.984075230
 8 : 1524.984076750
 9 : 1524.984078300
10 : 1524.984079840
11 : 1524.984081390
12 : 1524.984082920
13 : 1524.984084450
14 : 1524.984085980
15 : 1524.984087520
16 : 1524.984089030
17 : 1524.984090560
18 : 1524.984092070
19 : 1524.984093600
20 : 1524.984095120
21 : 1524.984096650
22 : 1524.984098180
23 : 1524.984099710
24 : 1524.984101230
25 : 1524.984102760
26 : 1524.984104300
27 : 1524.984105840
28 : 1524.984107380
29 : 1524.984108900

=== CLOCK_MONOTONIC ===
Theoretical resolution : 0.000000001 s
 0 : 1524.987991910
 1 : 1524.987993730
 2 : 1524.987995320
 3 : 1524.987996880
 4 : 1524.987998410
 5 : 1524.987999950
 6 : 1524.988001470
 7 : 1524.988002980
 8 : 1524.988004530
 9 : 1524.988006060
10 : 1524.988007600
11 : 1524.988009140
12 : 1524.988010680
13 : 1524.988012240
14 : 1524.988013790
15 : 1524.988015350
16 : 1524.988016880
17 : 1524.988018400
18 : 1524.988019960
19 : 1524.988021500
20 : 1524.988023060
21 : 1524.988024620
22 : 1524.988026160
23 : 1524.988027700
24 : 1524.988029230
25 : 1524.988030720
26 : 1524.988032280
27 : 1524.988033820
28 : 1524.988035340
29 : 1524.988036900
```

La résolution théorique retournée par `clock_getres()` est de 1 nanoseconde pour les horloges `CLOCK_REALTIME` et
`CLOCK_MONOTONIC`. Cependant, les mesures montrent que les écarts entre deux appels successifs sont de l’ordre de 1.5 à
2 microsecondes.

Cette différence s’explique par le fait que la résolution annoncée correspond à la granularité de l’interface, mais pas
à la précision réelle observable. En pratique, la mesure est limitée par le coût de l’appel système `clock_gettime()`,
l’ordonnancement du noyau Linux et la précision de l’horloge matérielle. On peut aussi noter que tous les temps affichés
sont donnés avec une précision de 10 nanosecondes.

Les deux horloges présentent des comportements similaires en termes de résolution, ce qui indique qu’elles reposent sur
une même source matérielle. La différence principale entre elles réside ailleurs. `CLOCK_REALTIME` peut être
modifiée, tandis que `CLOCK_MONOTONIC` est strictement croissante.

### Développement: timers

On a les informations suivantes.
- timer_handler: compte le nombre de signaux (timer expirés) et print sur stdout
- sigaction: réaffectation du signal `SIGVTALRM` pour lancer timer_handler
- it_value: configuration du timer pour expirer après 250ms
- it_interval: configuration du timer pour se reproduire toutes les 250ms
- setitimer: lancement du timer avec `ITIMER_VIRTUAL` lançant des `SIGVTALRM` en utilisant le temps cpu user-mode
- while(1): attente active pour faire fonctionner le programme

Ce code permet ainsi toutes les 250ms d'envoyer un message comme quoi le timer a expiré. On peut ainsi obtenir une 
tâche qui périodiquement va écrire dans notre shell.

### Modification

Voici un exemple d'output du programme pour 500us. La sortie est en ns.

```shell
475630
498000
498320
501100
498740
501410
498710
```

On voit donc qu'il y a des différences non négligeables entre chaque mesure. Probablement que celles-ci sont en moyenne
proche de la valeur espérée de 500us. Cette différence est probablement dûe au temps de réponse après avoir reçu le
signal qui fait donc varier les temps enregistrés. Même avec peu de code, il est impossible d'avoir des temps parfaits.

### Mesures

```shell
{lab02} schoki@Scarlet:~/Documents/HEIG-VD/Semestre4/PTR/ptr26/code$ ./summary < results/t500.dat
---------------------------------------------- summary1.c -----
  Total of 1000 values 
    Minimum  = 428840.000000 (position = 0) 
    Maximum  = 531400.000000 (position = 855) 
    Sum      = 499920320.000000 
    Mean     = 499920.320000 
    Variance = 33552557.697540 
    Std Dev  = 5792.456966 
    CoV      = 0.011587 
---------------------------------------------------------------
```

On voit bien qu'en moyenne, on tend vers 250us. Le problème est que notre première valeur est nettement en dessous. Cela
semble se confirmer pour les autres tests.

```shell
{lab02} schoki@Scarlet:~/Documents/HEIG-VD/Semestre4/PTR/ptr26/code$ ./summary < results/t250.dat
---------------------------------------------- summary1.c -----
  Total of 1000 values 
    Minimum  = 225420.000000 (position = 0) 
    Maximum  = 256260.000000 (position = 33) 
    Sum      = 249970950.000000 
    Mean     = 249970.950000 
    Variance = 1626254.997581 
    Std Dev  = 1275.247034 
    CoV      = 0.005102 
---------------------------------------------------------------
{lab02} schoki@Scarlet:~/Documents/HEIG-VD/Semestre4/PTR/ptr26/code$ ./summary < results/t1000.dat
---------------------------------------------- summary1.c -----
  Total of 1000 values 
    Minimum  = 973160.000000 (position = 0) 
    Maximum  = 1007870.000000 (position = 81) 
    Sum      = 999969770.000000 
    Mean     = 999969.770000 
    Variance = 1880639.044922 
    Std Dev  = 1371.363936 
    CoV      = 0.001371 
---------------------------------------------------------------
{lab02} schoki@Scarlet:~/Documents/HEIG-VD/Semestre4/PTR/ptr26/code$ ./summary < results/t10000.dat
---------------------------------------------- summary1.c -----
  Total of 1000 values 
    Minimum  = 9985980.000000 (position = 0) 
    Maximum  = 10016190.000000 (position = 600) 
    Sum      = 9999979610.000000 
    Mean     = 9999979.610000 
    Variance = 1157159.718750 
    Std Dev  = 1075.713586 
    CoV      = 0.000108 
---------------------------------------------------------------
```

La première position prend toujours le moins de temps. On peut aussi regarder les histogrammes pour mieux se rendre
compte de la dispersion.

![t250_hist.png](code/results/t250_hist.png)

![t500_hist.png](code/results/t500_hist.png)

![t1000_hist.png](code/results/t1000_hist.png)

![t10000_hist.png](code/results/t10000_hist.png)

Pour des raisons inconnues, il semble que la dispersion varie selon les histogrammes. Cela pourrait être dû à
l'utilisation du cpu à ce moment. Si le CPU est utilisé, il mettra plus ou moins de temps à répondre ce qui créera une
plus grande dispersion.

### Perturbations

```shell
nice -n -20 ./signal_timer2 1000 1000 > tnicelow.dat # low nice = high priority
```

![tnicelow_hist.png](code/results/tnicelow_hist.png)

```shell
nice -n 19 ./signal_timer2 1000 1000 > tnicehigh.dat # high nice = low priority
```

![tnicehigh_hist.png](code/results/tnicehigh_hist.png)

On remarque qu'un nice plus faible qui donne donc une meilleure priorité au process divise par 2 la différence entre le
maximum et le minimum. Cela permet de rester plus centré.

```shell
./cpu_loop & ./signal_timer2 1000 1000 > tcpuloop.dat
```

![tcpuloop_hist.png](code/results/tcpuloop_hist.png)

Avec un `cpu_loop` en fond, on peut noter que les performances sont catastrophiques. Pour une différence qui devrait
être autour des 1000us, on a une valeur à 9400us.

```shell
./signal_timer2 1000 1000 & ./signal_timer2 1000 1000 & ./signal_timer2 1000 1000 > ttriple.dat
```

![ttriple_hist.png](code/results/ttriple_hist.png)

J'ai essayé également de lancer trois fois le programme `signal_timer2` en même temps pour voir ce que cela fait. Comme
pour `cpu_loop`, on a une valeur autour de 9600us ce qui est loin des 1000us espérés.