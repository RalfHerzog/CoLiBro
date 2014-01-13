CoLiBro - Command Line Browser
==============================

Colibro is a lightweight C/C++ Library which supports receiving HTTP(S) Websites.

You can use it to:
------------------

* Download Websites and Files
* Make dynamic HTTP(S) requests directly out of C
* Save contents to file or wherever you want
* Parse HTML an get the nessesary data out of a page

This are the features:
----------------------

* Cookie handling
* Secure transaction (SSL via Polarssl)
* Support common compression methods (GZip, Deflate)
* Chunked transfer encoding
* Follow Webpages (Redirection)
* Post data to a Webside
* Fill a &lt;form&gt;-Tag with your personal data
* Generate post data from &lt;form&gt;-Tag
* Easy :-)
* Errorhandling (as far as possible)

You might ask, why not use wget or curl?

At the beginning I want to process website calls automatically. Especially the fact I could not parse an HTML page, lead me to start CoLiBro.

Documentation and Examples
--------------------------
Have a look at the example file(s). Documentation and more example files are planned. Due to small freetime my plannings may take some time.

Thanks
------
At this point I would like to thank the contributors of the external library's I use. They are:

* ZLib (Compression)
* PolarSSL (SSL implementation)
* FireDNS (DNS resolver)
* SQLite3 (Database)

Conclusion
----------
Remember this is just a hobby project. Feel free to report any issues and feature requests.