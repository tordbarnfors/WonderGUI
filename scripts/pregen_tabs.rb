
# unpackSRGBTab

    for i in 0..255

        srgb = i / 255.0;
        if( srgb < 0.04045 )
            linear = srgb / 12.92
        else
            linear = (srgb * 0.9478672986 + 0.0521327014) ** 2.4
        end

        linear = (linear * 4096.0).round

        if( i != 0 && i % 16 == 0 )
 #           print "\n"
        end

#        print "#{linear}, "
    end    

# packSRGBTab

    for i in 0..4096

        linear = i / 4096.0
        if(linear <=0.0031308)
            srgb = linear*12.92
        else
            srgb = (1+0.055)* (linear ** (1/2.4))-0.055
        end

        srgb = (srgb * 255.0).round

        if( i != 0 && i % 16 == 0 )
#           print "\n"
        end

#        print "#{srgb}, "
    end    

# unpackLinearTab

    for i in 0..255

        converted = (i / 255.0 * 4096.0).round

        if( i != 0 && i % 16 == 0 )
#           print "\n"
        end

#        print "#{converted}, "

    end

# packLinearTab

    for i in 0..4096

        converted = (i / 4096.0 * 255.0).round

        if( i != 0 && i % 16 == 0 )
#           print "\n"
        end

#        print "#{converted}, "

    end

# curveTab

    for i in 0..1023

        y = 1.0 - i / 1024.0

        out = (Math.sqrt(1.0 - y*y) * 65536).round

        if( i != 0 && i % 16 == 0 )
#           print "\n"
        end

 #       print "#{out}, "

    end

# line thickness tab

    for i in 0..16

        b = i / 16.0;
        out = (Math.sqrt(1.0 + b*b) * 65536).round;

        print"#{out}, "

    end



