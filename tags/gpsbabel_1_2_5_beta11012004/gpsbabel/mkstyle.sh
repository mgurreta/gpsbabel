
echo "/* This file is machine-generated from the contents of style/ */"
echo "/* by mkstyle.sh.   Editing it by hand is an exeedingly bad idea. */"
echo 

nstyles="0"
for i in style/*.style
do
	A=`basename $i | sed "s/.style$//"`
	[ $A = "README" ] && continue
	[ $A = "custom.style" ] && continue
	ALIST="{ \"$A\", $A } , $ALIST"
	echo "static char $A[] = "
	sed 's/\\/\\\\/;s/"/\\"/g;s/\(^.\)/"\1/g;s/\(.$\)/\1\\n"/g' $i
	echo ";"
	nstyles=`expr $nstyles + 1`;
done

echo "#include \"defs.h\""
echo "style_vecs_t style_list[] = {$ALIST {0,0}};"
echo "size_t nstyles = $nstyles;"
