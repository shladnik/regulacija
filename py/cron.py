#!/usr/bin/python3.2

f = open('index.html', 'w')

f.write('<?xml version="1.0" encoding="utf-8" ?>\n')
f.write('<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">\n')
f.write('<html xmlns="http://www.w3.org/1999/xhtml">\n')
f.write('<head>\n')
f.write('\t<title>Cron GUI</title>\n')
f.write('</head>\n')

f.write('<body>\n')
f.write('\t<form action="" method="get">\n')
f.write('\t\t<p>\n')
f.write('\t\t\t<label>Minute:')
f.write('\t\t\t\t<input type="text"></input>\n')
f.write('\t\t\t</label>\n')
f.write('\t\t</p>\n')
f.write('\t</form>\n')
f.write('</body>\n')
f.write('</html>')
