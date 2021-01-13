<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL bootstrap test suite
  :get-document-filename command 
-->
<Root xmlns:xpl="urn:x-xpl:xpl">
  <!-- we can't use the current version output with abs path -->
  <xpl:attribute name="dummy" destination="/dev/null">
    <xpl:get-document-filename/>
  </xpl:attribute>
</Root>
