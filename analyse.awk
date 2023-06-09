NR == 2 { old_srv = $1; old_rec = $2 ; print("reception emission") }
NR > 2 { srv = $1 ; rec = $2
    delta_srv = srv - old_srv ;
    delta_rec = rec - srv ;
    old_srv = srv;
    old_rec = rec;
    if (delta_rec > 1.0 || delta_rec < -1.0) {
	printf("delta_rec anormal %s\n", NR);
    }
    if (delta_srv > 1.0 || delta_srv < -1.0) {
	printf("delta_srv anormal %s\n", NR);
    }
    print(delta_rec, delta_srv)
}
