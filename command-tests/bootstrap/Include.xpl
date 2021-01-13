<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL bootstrap test suite
  :nclude command 
-->
<Root xmlns:xpl="urn:x-xpl:xpl">
  <Local attr="a">
    <xpl:include select="parent::*/@attr"/>
  </Local>

  <External>
    <xpl:include select="/Root/node()" file="xToInclude.xml"/>
  </External>
</Root>
