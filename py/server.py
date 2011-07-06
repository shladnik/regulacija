#!/usr/bin/python3

import gum
import xml.etree.ElementTree as etree
import cherrypy
import cherrypy.lib.auth_digest
import inspect

def diagonal_flip(table):
  h = len(table)
  tab = [ [] for i in table[0] ]
  for i in range(h):
    tab[i] = [ row[i] for row in table ]
  return tab


def build_table(data):
  table = etree.Element('table')
  table.attrib['border'] = '1'
  for rdata in data:
    row = etree.Element('tr')
    for cdata in rdata:
      cell = etree.Element('td')
      cell.text = str(cdata)
      row.append(cell)
    table.append(row)
  return table

def build_sensor_err_tab():
  slist = etree.parse("xml/xml.xml").getroot().find("ds18b20_list")
  table = [ [ d.attrib['cname'] ] for d in slist.findall("ds18b20") ]
  cnt = gum.read_symbol('ds18b20_err_cnt', None)
  rty = gum.read_symbol('ds18b20_max_rty', None)
  err_nr = int(len(cnt) / len(table))
  for i in range(len(table)):
    table[i] += cnt[i*err_nr:(i+1) * err_nr] +\
    rty[i*2:(i+1)*2]
  
  return build_table(table)


  

class Regulation(object):
  def skeleton(self, body):
    html = etree.Element('html')
    html.attrib['xmlns'] = 'http://www.w3.org/1999/xhtml'
    head = etree.Element('head')
    title = etree.Element('title')
    title.text = "Regulacija"
    head.append(title)
    html.append(head)
    body.append(etree.Element('hr'))
    link_tab = etree.Element('table')
    caller = inspect.stack()[1][3]
    tr = etree.Element('tr')
    for i in dir(self):
      if not i == caller:
        attr = eval('self.' + i)
        if hasattr(attr, 'link') and hasattr(attr, 'exposed') and attr.exposed:
          td = etree.Element('td')
          a = etree.Element('a')
          a.attrib['href'] = attr.__name__
          a.text = attr.link
          td.append(a)
          tr.append(td)
    link_tab.append(tr)
    body.append(link_tab)
    html.append(body)
    page = ""
    page += '<?xml version="1.0" encoding="UTF-8"?>'
    page += '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">'
    page += etree.tostring(html)
    return page

  def index(self):
    body  = etree.Element('body')
    table = etree.Element('table')

    cnt = 0
    table = []
    for d in etree.parse("xml/xml.xml").getroot().find("ds18b20_list").findall("ds18b20"):
      table[len(table):] = [ [ d.attrib['cname'], gum.ds18b20_get_temp(cnt), d.text ] ]
      cnt += 1
    body.append(build_table(table))
    
    cnt = 0
    table = []
    for d in etree.parse("xml/xml.xml").getroot().find("valve_list").findall("valve"):
      table[len(table):] = [ [ d.attrib['cname'], gum.valve_state(cnt), d.text ] ]
      cnt += 1
    body.append(build_table(table))

    cnt = 0
    table = []
    for d in etree.parse("xml/xml.xml").getroot().find("relay_list").findall("relay"):
      table[len(table):] = [ [ d.attrib['cname'], gum.relay_get(cnt), d.text ] ]
      cnt += 1
    body.append(build_table(table))

    return self.skeleton(body)

  def config(self, name=None, value=None):
    body = etree.Element("body")
    
    for s in etree.parse("xml/xml.xml").getroot().find("config_list").findall('*'):
      form = etree.Element("form")
      form.attrib['action'] = ""
      
      p = etree.Element("p")
      p.text = s.text.strip()
      namefield = etree.Element('input')
      namefield.attrib['type'] = 'hidden'
      namefield.attrib['name'] = 'name'
      namefield.attrib['value'] = s.tag
      p.append(namefield)
      
      sign = s.attrib['format'][0] == 's'
      dot  = s.attrib['format'].find('.')
      div = 1.0
      if dot >= 0:
        div = 2.0 ** int(s.attrib['format'][dot+1:])

      textfield = etree.Element('input')
      textfield.attrib['type'] = 'text'
      textfield.attrib['name'] = 'value'
      if name == s.tag:
        value = int(float(value) * div)
        gum.write_symbol(name, value)
      print('%x' % gum.read_symbol(s.tag))
      textfield.attrib['value'] = str(gum.read_symbol(s.tag) / div)
      p.append(textfield)

      submit = etree.Element('input')
      submit.attrib['type'] = 'submit'
      p.append(submit)
      form.append(p)

      body.append(form)

    return self.skeleton(body)
    
  
  def devel(self, name=None, val=None, arg0="", arg1="", arg2="", arg3="", *args):
    body = etree.Element('body')
   
    form   = etree.Element("form")
    form.attrib['action'] = ""

    fieldset = etree.Element("fieldset")
    select = etree.Element("select")
    keys = list(gum.symbols.keys())
    keys.sort()
    for s in keys:
      if gum.symbols[s]['section'] != ".text" and \
         gum.symbols[s]['size'] > 0:
        opt = etree.Element("option")
        opt.attrib['value'] = s
        opt.text = s
        if name == s: opt.attrib['selected'] = 'selected'
        select.append(opt)
    select.attrib['name'] = 'name'
    select.attrib['onchange'] = 'this.form.val.value=""; this.form.submit();'
    fieldset.append(select)
    
    textfield = etree.Element('input')
    textfield.attrib['type'] = 'text'
    textfield.attrib['name'] = 'val'
   
    if name:
      if name[0:2] == '0x':
        adr = int(name[2:], 0x10)
        if val:
          if val[0:2] == '0x':
            val = int(val[2:], 0x10)
          else:
            val = int(val)
          print(adr, val)
          gum.rs232.access(1, adr, bytearray([val]))
        textfield.attrib['value'] = hex(gum.rs232.access(0, adr, bytearray(1))[0])
      elif gum.symbols[name]['section'] != ".text" and \
           gum.symbols[name]['size'] > 0:
        if val:
          if val[0:2] == '0x':
            val = int(val[2:], 0x10)
          else:
            val = int(val)
          gum.write_symbol(name, val)
        textfield.attrib['value'] = hex(gum.read_symbol(name))

    fieldset.append(textfield)

    form.append(fieldset)
    body.append(form)
    
    #
    # exexec
    #
    form = etree.Element("form")
    form.attrib['action'] = ""

    fieldset = etree.Element("fieldset")
    select = etree.Element("select")
    keys = list(gum.symbols.keys())
    keys.sort()
    for s in keys:
      if gum.symbols[s]['section'] == ".text" and \
         gum.symbols[s]['size'] > 0:
        opt = etree.Element("option")
        opt.attrib['value'] = s
        opt.text = s
        if name == s: opt.attrib['selected'] = 'selected'
        select.append(opt)
    select.attrib['name'] = 'name'
    select.attrib['onchange'] = '''
    this.form.arg0.value="";
    this.form.arg1.value="";
    this.form.arg2.value="";
    this.form.arg3.value="";
    '''
    fieldset.append(select)
    
    textfield = etree.Element('input')
    textfield.attrib['type'] = 'text'
    textfield.attrib['name'] = 'arg0'
    textfield.attrib['size'] = '4'
    if arg0 == "":
      arg0 = 0
    elif arg0[0:2] == '0x':
      arg0 = int(arg0[2:], 0x10)
    else:
      arg0 = int(arg0)
    textfield.attrib['value'] = str(arg0)
    fieldset.append(textfield)
    textfield = etree.Element('input')
    textfield.attrib['type'] = 'text'
    textfield.attrib['name'] = 'arg1'
    textfield.attrib['size'] = '4'
    if arg1 == "":
      arg1 = 0
    elif arg1[0:2] == '0x':
      arg1 = int(arg1[2:], 0x10)
    else:
      arg1 = int(arg1)
    textfield.attrib['value'] = str(arg1)
    fieldset.append(textfield)
    textfield = etree.Element('input')
    textfield.attrib['type'] = 'text'
    textfield.attrib['name'] = 'arg2'
    textfield.attrib['size'] = '4'
    if arg2 == "":
      arg2 = 0
    elif arg2[0:2] == '0x':
      arg2 = int(arg2[2:], 0x10)
    else:
      arg2 = int(arg2)
    textfield.attrib['value'] = str(arg2)
    fieldset.append(textfield)
    textfield = etree.Element('input')
    textfield.attrib['type'] = 'text'
    textfield.attrib['name'] = 'arg3'
    textfield.attrib['size'] = '4'
    if arg3 == "":
      arg3 = 0
    elif arg3[0:2] == '0x':
      arg3 = int(arg3[2:], 0x10)
    else:
      arg3 = int(arg3)
    textfield.attrib['value'] = str(arg3)
    fieldset.append(textfield)

    if name and not name[0:2] == '0x' and gum.symbols[name]['section'] == ".text":
      textfield = etree.Element('input')
      textfield.attrib['type'] = 'text'
      textfield.attrib['name'] = 'return'
      textfield.attrib['disabled'] = 'disabled'
      ret_val = gum.exexec(name, [arg0, arg1, arg2, arg3])
      textfield.attrib['value'] = ""
      for i in ret_val:
        textfield.attrib['value'] += "%4x " % i
      fieldset.append(textfield)
    
    submit = etree.Element('input')
    submit.attrib['type'] = 'submit'
    fieldset.append(submit)

    form.append(fieldset)
    body.append(form)
    
    tr = etree.Element('tr')
    
    #
    # Console
    #
    console = gum.readcon()
    if len(console):
      td = etree.Element('td')
      textarea = etree.Element('textarea')
      textarea.attrib['cols'] = '40'
      textarea.attrib['rows'] = '20'
      textarea.attrib['disabled'] = 'disabled'
      textarea.text = console
      td.append(textarea)
      tr.append(td)
   
    #
    # dbg_* vars
    #
    td = etree.Element('td')
    dbg_tab = etree.Element('table')
    for s in sorted(gum.symbols):
      if gum.symbols[s]['section'] == ".dbg" and \
         gum.symbols[s]['size'] > 0 and \
         s != 'bla' and \
         s != 'ds18b20_err_cnt' and \
         s != 'ds18b20_max_rty' and \
         s[0:9] != 'print_buf':
        dbg = etree.Element("tr")
        dbgn = etree.Element("td")
        dbgn.text = s
        dbg.append(dbgn)
        dbgv = etree.Element("td")
        dbgv.text = hex(gum.read_symbol(s))
        dbg.append(dbgv)
        dbg_tab.append(dbg)
    td.append(dbg_tab)

    tr.append(td)

    table = etree.Element('table')
    table.append(tr)
   
    body.append(table)
    body.append(build_sensor_err_tab())

    #bla = gum.read_symbol('bla', None);
    #table = []
    #for i in range(0, len(bla), 0x80):
    #  table[len(table):] = [ bla[i:i+0x80] ]
    #body.append(build_table(table))

    #bla = gum.read_symbol('timer_now_log', None);
    #table = []
    #for i in range(0, len(bla), 0x4):
    #  table[len(table):] = [ [ gum.to_int(bla[i:i+0x4]) ] ]
    #body.append(build_table(table))
    
    return self.skeleton(body)

  index.__dict__ = {
    'exposed' : True,
    'link'    : "Prva stran",
  }
  
  config.__dict__ = {
    'exposed' : True,
    'link'    : "Nastavitve",
  }
  
  devel.__dict__ = {
    'exposed' : True,
    'link'    : "Razvoj",
  }




  #config._cp_config = devel._cp_config = {
  #  'tools.auth_digest.on'      : True,
  #  'tools.auth_digest.realm'   : 'home',
  #  'tools.auth_digest.get_ha1' : cherrypy.lib.auth_digest.get_ha1_file_htdigest('security/htdigest'),
  #  'tools.auth_digest.key'     : 'a565c27146791cfb',
  #}


#def to_https():
#  url = cherrypy.url()
#  print(url)
#  if url[0:4] == 'http' and url[4] != 's':
#    url = url[0:4] + 's' + url[4:]
#    print("Redirect to", url)
#    raise cherrypy.HTTPRedirect(url)
#
#cherrypy.tools.to_https = cherrypy._cptools.HandlerTool(to_https)

cherrypy.quickstart(Regulation(), '', {
  'global' : {
    #'engine.autoreload_on'      : False,
    #'server.socket_host'        : 'stefuc.homeip.net',
    'server.socket_host'        : '0.0.0.0',
    'server.socket_port'        : 8000,
    'tools.sessions.on'         : False,
    'tools.gzip.on'             : False,
    # SSL 
    #'tools.to_https.on'         : True, # doesn't work...
    #'server.ssl_certificate'    : 'security/home_cert.crt',
    #'server.ssl_private_key'    : 'security/home_cert.key',
    # Authentication
    'tools.auth_digest.on'      : True,
    'tools.auth_digest.realm'   : 'home',
    'tools.auth_digest.get_ha1' : cherrypy.lib.auth_digest.get_ha1_file_htdigest('security/htdigest'),
    'tools.auth_digest.key'     : 'a565c27146791cfb',
  },
})


