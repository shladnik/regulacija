#!/usr/bin/python3

import xml.etree.ElementTree
import cherrypy
import cherrypy.lib.auth_digest
import inspect
import time
import sys
import os
import threading

import gum
import tools
import html_tools
import packer

el = xml.etree.ElementTree.Element


#if len(sys.argv) > 1:
#  baud = int(sys.argv[1])
#else:
#  baud = None
#
#if len(sys.argv) > 2:
#  meta = sys.argv[2]
#else:
#  meta = None



def diagonal_flip(table):
  h = len(table)
  tab = [ [] for i in table[0] ]
  for i in range(h):
    tab[i] = [ row[i] for row in table ]
  return tab


def build_table(data):
  table = el('table')
  table.attrib['border'] = '1'
  for rdata in data:
    row = el('tr')
    for cdata in rdata:
      cell = el('td')
      cell.text = str(cdata)
      row.append(cell)
    table.append(row)
  return table

def build_sensor_err_tab(gumi):
  slist = xml.etree.ElementTree.parse("xml/xml.xml").getroot().find("ds18b20_list")
  table = [ [ d.attrib['cname'] ] for d in slist.findall("ds18b20") ]
  cnt = gumi.read_symbol('ds18b20_err_cnt', None)
  rty = gumi.read_symbol('ds18b20_max_rty', None)
  err_nr = int(len(cnt) / len(table))
  for i in range(len(table)):
    table[i] += cnt[i*err_nr:(i+1) * err_nr] +\
    rty[i*2:(i+1)*2]
  
  return build_table(table)


  

class Regulation(object):
  def skeleton(self, body, gumi = None):
    html = el('html')
    html.attrib['xmlns'] = 'http://www.w3.org/1999/xhtml'
    head = el('head')
    title = el('title')
    title.text = "Regulacija"
    head.append(title)
    html.append(head)
    body.append(el('hr'))
    link_tab = el('table')
    caller = inspect.stack()[1][3]
    tr = el('tr')
    for i in dir(self):
      if not i == caller:
        attr = eval('self.' + i)
        if hasattr(attr, 'link') and hasattr(attr, 'exposed') and attr.exposed:
          td = el('td')
          a = el('a')
          a.attrib['href'] = attr.__name__
          a.text = attr.link
          td.append(a)
          tr.append(td)
    link_tab.append(tr)
    body.append(link_tab)
    body.append(el('hr'))
   
    if gumi:
      times = el('p')
      times.text = ""
      
      daylist   = [ 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun' ]
      monthlist = [ 'Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec' ]
      date = gumi.read_symbol('date', None)
      
      times.text += "%s, %d %s %2d " % (daylist[date[3]], date[4] + 1, monthlist[date[5]], tools.to_int(date[6:8])) # date
      times.text += "%d:%02d:%02d " % (date[2], date[1], date[0])
      
      uptime = gumi.read_symbol('uptime')
      up_days   = uptime / (60 * 60 * 24)
      uptime   %= (60 * 60 * 24)
      up_hours  = uptime / (60 * 60)
      uptime   %= (60 * 60)
      up_mins  = uptime / 60 
      uptime   %= 60

      times.text += "(Uptime: %dd %dh %dm %ds)" % (up_days, up_hours, up_mins, uptime)
      body.append(times)

    html.append(body)
    page = ""
    page += '<?xml version="1.0" encoding="UTF-8"?>'
    page += '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">'
    page += xml.etree.ElementTree.tostring(html)
    return page

  def index(self):
    gumi = gum.Gum()
    body  = el('body')
    table = el('table')

    cnt = 0
    table = []
    temp_tab = gumi.ds18b20_get_temp()
    for d in xml.etree.ElementTree.parse("xml/xml.xml").getroot().find("ds18b20_list").findall("ds18b20"):
      table[len(table):] = [ [ d.attrib['cname'], temp_tab[cnt], d.text ] ]
      cnt += 1
    body.append(build_table(table))
    
    cnt = 0
    table = []
    for d in xml.etree.ElementTree.parse("xml/xml.xml").getroot().find("valve_list").findall("valve"):
      table[len(table):] = [ [ d.attrib['cname'], gumi.valve_state(cnt), d.text ] ]
      cnt += 1
    body.append(build_table(table))

    cnt = 0
    table = []
    for d in xml.etree.ElementTree.parse("xml/xml.xml").getroot().find("relay_list").findall("relay"):
      table[len(table):] = [ [ d.attrib['cname'], gumi.relay_get(cnt), d.text ] ]
      cnt += 1
    body.append(build_table(table))

    return self.skeleton(body, gumi)

  def config(self, name=None, value=None):
    gumi = gum.Gum()
    body = el("body")
    
    for s in xml.etree.ElementTree.parse("xml/xml.xml").getroot().find("config_list").findall('*'):
      form = el("form")
      form.attrib['action'] = ""
      
      p = el("p")
      p.text = s.text.strip()
      namefield = el('input')
      namefield.attrib['type'] = 'hidden'
      namefield.attrib['name'] = 'name'
      namefield.attrib['value'] = s.tag
      p.append(namefield)
      
      sign = s.attrib['format'][0] == 's'
      dot  = s.attrib['format'].find('.')
      size = int(s.attrib['format'][sign:dot]) + int(s.attrib['format'][dot+1:])
      div = 1.0
      if dot >= 0:
        div = 2.0 ** int(s.attrib['format'][dot+1:])

      textfield = el('input')
      textfield.attrib['type'] = 'text'
      textfield.attrib['name'] = 'value'
      if name == s.tag:
        value = int(float(value) * div)
        gumi.write_symbol(name, value)
      rval = int(gumi.read_symbol(s.tag))
      if rval >= (1 << size): rval -= 2 << size
      textfield.attrib['value'] = str(1.0*rval / div)
      p.append(textfield)

      submit = el('input')
      submit.attrib['type'] = 'submit'
      p.append(submit)
      form.append(p)

      body.append(form)
    
    # sync time button
    form = el("form")
    form.attrib['action'] = ""
    
    p = el("p")

    p.text = time.ctime()
    namefield = el('input')
    namefield.attrib['type'] = 'hidden'
    namefield.attrib['name'] = 'name'
    namefield.attrib['value'] = 'set_time'
    p.append(namefield)
    
    valfield = el('input')
    valfield.attrib['type'] = 'hidden'
    valfield.attrib['name'] = 'value'
    valfield.attrib['value'] = ''
    p.append(valfield)
    
    if (name == namefield.attrib['value']):
      if value: gumi.set_time(value)
      else:     gumi.set_time()


    submit = el('input')
    submit.attrib['type'] = 'submit'
    submit.attrib['value'] = 'Sinhroniziraj cas'
    p.append(submit)
    form.append(p)

    body.append(form)

    return self.skeleton(body, gumi)
    
  
  def devel(self, ram_symbol= None,
                  ram_val   = None, exexec_symbol= None,
                                    exexec_arg0  = None,
                                    exexec_arg1  = None,
                                    exexec_arg2  = None,
                                    exexec_arg3  = None):
    gumi = gum.Gum()
    body = el('body')
  
    #
    # RAM Symbols
    #
    form   = el("form")
    form.attrib['action'] = ""
    fieldset = el("fieldset")

    ram_list = filter(lambda x: gumi.meta['symbols'][x]['size'] > 0, gumi.meta['symbols'].keys())
    ram_list = filter(lambda x: gumi.meta['symbols'][x]['mem'] == "ram", ram_list)
    ram_list = list(ram_list)
    ram_list.sort()
    
    fieldset.append(html_tools.select(ram_list, "ram_symbol", selected = ram_symbol,
                                      onchange = 'this.form.' + 'ram_val' + '.value=""; this.form.submit();'))

    val = ""
    if ram_symbol in ram_list:
      if ram_val:
        if ram_val[0:2] == '0x':
          ram_val = int(ram_val[2:], 0x10)
        else:
          ram_val = int(ram_val)
        gumi.write_symbol(ram_symbol, ram_val)
      val = hex(gumi.read_symbol(ram_symbol))

    fieldset.append(el('input', type = 'text', name = 'ram_val', value = val))

    form.append(fieldset)
    body.append(form)
    
    #
    # exexec
    #
    exexec_list = filter(lambda x: gumi.meta['symbols'][x]['mem'] == 'flash', gumi.meta['symbols'])
    exexec_list = filter(lambda x: gumi.meta['symbols'][x]['size'], exexec_list)
    exexec_list = filter(lambda x: gumi.meta['symbols'][x]['section'] == '.text' or gumi.meta['symbols'][x]['section'] == '.flash_write', exexec_list)
    exexec_list = list(exexec_list)
    exexec_list.sort()

    form = el("form", action = "")
    fieldset = el("fieldset")
    
    argn = list(map(lambda x: 'exexec_arg' + str(x), range(4)))
    jscript = ''.join(map(lambda x: 'this.form.' + x + '.value="";', argn))
    fieldset.append(html_tools.select(exexec_list, "exexec_symbol", selected = exexec_symbol, onchange = jscript))

    def make_val(arg):
      if not arg:            arg = 0
      elif arg[0:2] == '0x': arg = int(arg[2:], 0x10)
      else:                  arg = int(arg)
      return str(arg)
    
    for a in argn: fieldset.append(el('input', type = 'text', size = str(4), name = a, value = make_val(eval(a))))
    
    if exexec_symbol in exexec_list:
      for i in gumi.exexec(exexec_symbol, list(map(eval, argn))):
        fieldset.append(el("input", type = 'text', size = str(4), value = hex(i), disabled="disabled"))

    submit = el('input')
    submit.attrib['type'] = 'submit'
    fieldset.append(submit)

    form.append(fieldset)
    body.append(form)
    
    p = el('p')
    p.text = "Stack status: " + str(gumi.exexec('stack_check')[0])
    body.append(p)
    
    tr = el('tr')
    
    #
    # Console
    #
    console = gumi.readcon()
    if len(console) :
      td = el('td')
      textarea = el('textarea', cols = '40', rows = '20', disabled = 'disabled')
      textarea.text = console
      td.append(textarea)
      tr.append(td)
    
    #
    # .dbg vars
    #
    dbg_list = filter(lambda x: gumi.meta['symbols'][x]['section'] == ".dbg", gumi.meta['symbols'])
    dbg_list = filter(lambda x: gumi.meta['symbols'][x]['size'], dbg_list)
    dbg_list = filter(lambda x: x != 'ds18b20_err_cnt', dbg_list)
    dbg_list = filter(lambda x: x != 'ds18b20_max_rty', dbg_list)
    dbg_list = filter(lambda x: x[0:9] != 'print_buf', dbg_list)
    dbg_list = list(dbg_list)
    dbg_list.sort()
    
    td = el('td')
    td.append(html_tools.table(list(map(lambda x: [ x, hex(gumi.read_symbol(x)) ], dbg_list))))
    tr.append(td)
    
    # .dbgcp
    dbgcp_list = filter(lambda x: gumi.meta['symbols'][x]['mem'] == "ram", gumi.meta['symbols'])
    dbgcp_list = filter(lambda x: gumi.meta['symbols']['__dbg2cp_start']['adr'] <= gumi.meta['symbols'][x]['adr'] < gumi.meta['symbols']['__dbg2cp_end']['adr'], dbgcp_list)
    dbgcp_list = filter(lambda x: gumi.meta['symbols'][x]['size'], dbgcp_list)
    dbgcp_list = list(dbgcp_list)
    dbgcp_list.sort()
    td = el('td')
    td.append(html_tools.table(list(map(lambda x: [ x, hex(gumi.read_symbol_dbgcp(x)) ], dbgcp_list))))
    tr.append(td)

    table = el('table')
    table.append(tr)
   
    body.append(table)
    body.append(build_sensor_err_tab(gumi))

    return self.skeleton(body, gumi)

  def flash(self, fw_bin = None, bootloader_bin = None, update = None):
    body = el('body')

    #form = el("form")
    #form.text = "Firmware:"
    #form.attrib['method'] = 'post'
    #form.attrib['enctype'] = 'multipart/form-data'
    #file_input = el("input")
    #file_input.attrib['type'] = 'file'
    #file_input.attrib['name'] = 'fw_bin'
    #form.append(file_input)
    #submit = el("input")
    #submit.attrib['type'] = 'submit'
    #form.append(submit)
    #body.append(form)
    #
    #if fw_bin:
    #  body.text = "Flashed fw with " + fw_bin.filename
    #  gumi.flash_fw(fw_bin.file)
    #
    #form = el("form")
    #form.text = "Bootloader:"
    #form.attrib['method'] = 'post'
    #form.attrib['enctype'] = 'multipart/form-data'
    #file_input = el("input")
    #file_input.attrib['type'] = 'file'
    #file_input.attrib['name'] = 'bootloader_bin'
    #form.append(file_input)
    #submit = el("input")
    #submit.attrib['type'] = 'submit'
    #form.append(submit)
    #body.append(form)
    #
    #if bootloader_bin:
    #  body.text = "Flashed bootloader with " + bootloader_bin.filename
    #  gumi.flash_bootloader(bootloader_bin.file)

    if update:
      body.text = "Applying " + update.filename
      sf = open(update.filename, mode='wb')
      sf.write(update.file.read())
      sf.close()

      def update_thread():
        packer.update(update.filename)
        time.sleep(1)
        os.execv(sys.argv[0], sys.argv)
      
      threading.Thread(target = update_thread).start()
    else:
      form = el("form")
      form.text = "Update:"
      form.attrib['method'] = 'post'
      form.attrib['enctype'] = 'multipart/form-data'
      file_input = el("input")
      file_input.attrib['type'] = 'file'
      file_input.attrib['name'] = 'update'
      form.append(file_input)
      submit = el("input")
      submit.attrib['type'] = 'submit'
      form.append(submit)
      body.append(form)
    


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

  flash.__dict__ = {
    'exposed' : True,
    'link'    : "Flash",
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
    'engine.autoreload_on'      : False,
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

