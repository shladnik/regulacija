#!/usr/bin/python3

# id:[element, type, label]
elmnts = {	'weekdays':['select', ['mon', 'tue', 'wed', 'thu', 'fri', 'sat', 'sun'], 'Day:'],
		'min':['input', 'text', 'Minutes:'],
		'hrs':['input', 'text', 'Hours:'],
		'month':['input', 'text', 'Month:'],
		'day':['input', 'text', 'Day:']
	}

f = open('index.html', 'w')

def htmlInput( inputId, inputType, inputLabel ):
	htmlstr = ''

	htmlstr = '<p>\n'
	htmlstr += '\t<label>' + inputLabel
	htmlstr += '<input id="' + inputId + '" name="' + inputId + '" type="' + inputType + '" />'
	htmlstr += '</label>\n'
	htmlstr += '</p>\n'
	
	return htmlstr

def htmlSelect( inputId, options, inputLabel ):
	htmlstr = ''	

	htmlstr = '<p>\n'
	htmlstr += '\t<label>' + inputLabel + '\n'
	htmlstr += '\t\t<select id="' + inputId + '" name="' + inputId + '">\n'
	
	for opt in options:
		htmlstr += '\t\t\t<option value="' + opt + '">' + opt + '</option>\n'	

	htmlstr += '\t\t</select>\n'
	htmlstr += '\t</label>\n'
	htmlstr += '</p>\n'
	
	return htmlstr


f.write('<?xml version="1.0" encoding="utf-8" ?>\n')
f.write('<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">\n')
f.write('<html xmlns="http://www.w3.org/1999/xhtml">\n')
f.write('<head>\n')
f.write('\t<title>Cron GUI</title>\n')
f.write('</head>\n')

f.write('<body>\n')
f.write('\t<form action="" method="get">\n')

for key, val in elmnts.items():
	if val[0].lower() == 'input':
		f.write( htmlInput( key, val[1], val[2] ) )
	elif val[0].lower() == 'select':
		f.write( htmlSelect( key, val[1], val[2] ) )

f.write('<input type="submit" title="Submit" />')
f.write('\t</form>\n')
f.write('</body>\n')
f.write('</html>')
