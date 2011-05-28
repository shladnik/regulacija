#!/usr/bin/python3

import xml.etree.ElementTree
import http.server
import gum

class CustomHTTPRequestHandler(http.server.BaseHTTPRequestHandler):
  def do_GET(self):
    self.send_response(200)
    self.send_header('Content-type', 'text/html')
    self.end_headers()
    #print("rfile")
    #print(self.rfile.read(100))
    #print("rfile")
    
    
    
    msg = ""

    slist = xml.etree.ElementTree.parse("xml/xml.xml").getroot().find("ds18b20_list")
    cnt = 0
    for sensor in slist.findall("ds18b20"):
      msg += str(gum.exexec_ds18b20_get_temp(cnt))
      msg += " "
      msg += sensor.text.strip()
      msg += ' <br>'
      msg += '\n'
      cnt += 1

    print(msg)
    msg = bytearray(msg, encoding='utf-8')
    self.wfile.write(msg)
    
    
    return

httpd = http.server.HTTPServer(('', 8000), CustomHTTPRequestHandler)
httpd.serve_forever()
