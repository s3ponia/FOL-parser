5
@ vA . @ vB . ( pdisjoint(vA,vB) -> pdisjoint(vB,vA) )
@ vA . @ vB . ( pin(vA,vB) -> ~ pin(vB,vA) )
? vA . pempty(vA)
? vA . ~ pempty(vA)
@ vA . @ vB . ( ~ pin(vA,vB) -> pdisjoint(fsingleton(vA),vB) )
@ vA . @ vB . ( ~ pin(vA,vB) -> pdisjoint(fsingleton(vA),vB) )
