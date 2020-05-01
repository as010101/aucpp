#!/usr/bin/perl

# The field types are (see TupleDescription.H
# int 7
# float 8
# double 9
# string 10

my $nfields, $bytespertuple, $done1 = 0, @fields;

my %types = (7, "int",
	     8, "float",
	     9, "double",
	     10, "string");

my $filename = @ARGV[0];

while (<>) {
    chomp;
    # skip over comments
    if (/^\#/) {
    } else {
	if (!$done1) {
	    # first line read is the number of fields, and the bytes per tuple
	    ($nfields,$bytespertuple) = split(/,/);
	    print "Queue contains $nfields fields, tuples of size $bytespertuple\n";
	    $done1 = 1;
	} else {
	# Finish off with the other lines
	    ($fieldtype,$bytes) = split(/,/);
	    #print "Found field type ";
	    #print $types{$fieldtype};
	    #print " of lengh $bytes byte(s)\n";
	    push(@fields, [$fieldtype, $bytes]);
	}
    }
}
# To print the array of arrays
#foreach my $ref(@fields) {
#    print "this consists of: ";
#    foreach(@$ref) {
#	print "$_ ";
#    }
#    print "\n";
#}


# Now print the file by using od

# Count how many tuples we have (file size / bytes per tuple)

# Now our file is not the description file
$filename =~ s/.desc//;
my $ntuples = (-s $filename) / $bytespertuple;

# This way would be safer if the queue file is cut or anything wierd
#$ntuples = 0;
#open(QUEUE,"queue.000");
#binmode(QUEUE);
#my $buf;
#while (sysread(QUEUE, $buf, $bytespertuple) == $bytespertuple) {
#    $ntuples++;
#}

print "Queue contains $ntuples tuples\n";
$pos = 0; # current position in bytes in the queue
$t = 0;
while ($t < $ntuples) {
    print "--- TUPLE ---\n";
    foreach my $ref(@fields) { # for each field
	# you now know the type and length of each
	$type = @$ref[0];
	$len = @$ref[1];
	
	if ($type == 7) { # integer
	    print "Integer: ";
	    system "od -A n -t d4 -j $pos -N $len $filename";
	    $pos += $len;
	} elsif ($type == 10) { # characters
	    print "String: ";
	    system "od -A n -t c -j $pos -N $len $filename";
	    $pos += $len;
	}
    }
    $t++;
}
exit;


# To do it in perl (INCOMPLETE FOR NOW)
# Now read the queue file
open(QUEUE,"queue.000");
binmode(QUEUE);
my $bytesread, $buf;

foreach my $ref(@fields) { # for each field
    # you now know the type and length of each
    $type = @$ref[0];
    $len = @$ref[1];
    print "Field: type $type, length $len\n";

    # so now you can read just that field
    sysread(QUEUE, $buf, $len) == $len or die "Short read - end of queue";

    # and print it out appropriately
    if ($type == 7) { # int
	printf("int: %d\n", $buf);
    } elsif ($type == 10) { # string, which is NOT null terminated! print char by char
	$i = 0;
	print "string: ";
	while ($i < $len) {
	    print "printing a char";
	    printf("[%c]", $buf);
	    $i++;
	}
	print "\n";
    }
}

exit;
# Read bytes per field
$i = 0;
while ($i < @fields) {
    ($type,$len) = pop(@fields);
    #$bytesread = sysread(QUEUE,$buf,
    print "Field: type $type, length $len\n";
    $i++;
}
#while ($bytesread = sysread(QUEUE,$buf,$bytespertuple)) {
#    print "read one tuple\n";
#    print "first byte is [";
#    print $buf[1];
#    print "]\n";
#}
