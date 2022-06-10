5
@ vA . @ vB . psubset(vA,vA)
@ vA . @ vB . ( pin(vA,vB) -> ~ pin(vB,vA) )
? vA . pempty(vA)
? vA . ~ pempty(vA)
@ vA . @ vB . @ vC . ( psubset(vA,vB) -> ( pin(vC,vA) or psubset(vA,fset_difference(vB,fsingleton(vC))) ) )
@ vA . @ vB . @ vC . ( psubset(vA,vB) -> ( pin(vC,vA) or psubset(vA,fset_difference(vB,fsingleton(vC))) ) )
