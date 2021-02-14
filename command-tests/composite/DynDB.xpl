<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :add-db, :change-db, :get-db and :remove-db commands
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/unchecked">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:add-db name="somedb" check="false">whatever</xpl:add-db>
      <xpl:change-db name="somedb" check="false">new_connstring</xpl:change-db>
      <Single>
        <xpl:get-db name="somedb"/>
      </Single>
      <Multiple>
        <xpl:include select="database[@name='somedb']">
          <xpl:get-db/>
        </xpl:include>
      </Multiple>
      <MultipleCustomTagName>
        <xpl:include select="ns-a:db[@name='somedb']">
          <xpl:get-db tagname="ns-a:db"/>
        </xpl:include>
      </MultipleCustomTagName>
      <MultipleShowTags>
        <xpl:include select="somedb">
          <xpl:get-db showtags="true"/>
        </xpl:include>
      </MultipleShowTags>
      <xpl:remove-db name="somedb"/>
    </Input>
    <Expected>
      <Single>new_connstring</Single>
      <Multiple>
        <database name="somedb">new_connstring</database>
      </Multiple>
      <MultipleCustomTagName>
        <ns-a:db name="somedb">new_connstring</ns-a:db>
      </MultipleCustomTagName>
      <MultipleShowTags>
        <somedb>new_connstring</somedb>
      </MultipleShowTags>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/add-db-modify-if-exists">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:add-db name="somedb">old_connstring</xpl:add-db>
      <xpl:add-db name="somedb" modifyifexists="true">new_connstring</xpl:add-db>
      <xpl:get-db name="somedb"/>
      <xpl:remove-db name="somedb"/>
    </Input>
    <Expected>new_connstring</Expected>
  </MustSucceed>

  <MustSucceed name="pass/change-db-add-if-not-exists">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:change-db name="somedb" addifnotexists="true">new_connstring</xpl:change-db>
      <xpl:get-db name="somedb"/>
      <xpl:remove-db name="somedb"/>
    </Input>
    <Expected>new_connstring</Expected>
  </MustSucceed>

  <MustSucceed name="pass/remove-db-ignore-if-not-exists">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:remove-db name="heffalump" ignoreifnotexists="true"/>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustFail name="fail/add-db-empty-conn-string">
    <Input>
      <xpl:add-db name="db"/>
    </Input>
  </MustFail>

  <MustFail name="fail/add-db-no-name">
    <Input>
      <xpl:add-db>whatever</xpl:add-db>
    </Input>
  </MustFail>

  <MustFail name="fail/add-db-bad-name">
    <Input>
      <xpl:add-db name="#">whatever</xpl:add-db>
    </Input>
  </MustFail>

  <MustFail name="fail/add-db-bad-modify-if-exists">
    <Input>
      <xpl:add-db name="db" modifyifexists="maybe">whatever</xpl:add-db>
    </Input>
  </MustFail>

  <MustFail name="fail/add-db-bad-check">
    <Input>
      <xpl:add-db name="db" check="relaxed">whatever</xpl:add-db>
    </Input>
  </MustFail>

  <MustFail name="fail/add-db-access-denied">
    <Input>
      <xpl:add-db name="db" check="false">whatever</xpl:add-db>
    </Input>
  </MustFail>

  <MustFail name="fail/add-db-already-exists">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:add-db name="somedb">old_connstring</xpl:add-db>
      <xpl:add-db name="somedb">new_connstring</xpl:add-db>
      <xpl:remove-db name="somedb"/>
    </Input>
  </MustFail>

  <MustFail name="fail/change-db-empty-conn-string">
    <Input>
      <xpl:change-db name="db"/>
    </Input>
  </MustFail>

  <MustFail name="fail/change-db-no-name">
    <Input>
      <xpl:change-db>whatever</xpl:change-db>
    </Input>
  </MustFail>

  <MustFail name="fail/change-db-bad-name">
    <Input>
      <xpl:change-db name="#">whatever</xpl:change-db>
    </Input>
  </MustFail>

  <MustFail name="fail/change-db-bad-add-if-not-exists">
    <Input>
      <xpl:change-db name="db" addifnotexists="maybe">whatever</xpl:change-db>
    </Input>
  </MustFail>

  <MustFail name="fail/change-db-bad-check">
    <Input>
      <xpl:change-db name="db" check="relaxed">whatever</xpl:change-db>
    </Input>
  </MustFail>

  <MustFail name="fail/change-db-access-denied">
    <Input>
      <xpl:change-db name="db" check="false">whatever</xpl:change-db>
    </Input>
  </MustFail>

  <MustFail name="fail/change-db-not-exists">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:change-db name="heffalump">whatever</xpl:change-db>
    </Input>
  </MustFail>

  <MustFail name="fail/get-db-bad-name">
    <Input>
      <xpl:get-db name="#"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-db-bad-tag-name">
    <Input>
      <xpl:get-db tagname="#"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-db-bad-show-tags">
    <Input>
      <xpl:get-db showtags="maybe"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-db-bad-repeat">
    <Input>
      <xpl:get-db repeat="twice"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-db-show-tags-and-tag-name">
    <Input>
      <xpl:get-db tagname="db" showtags="true"/>
    </Input>
  </MustFail>

  <MustFail name="fail/remove-db-no-name">
    <Input>
      <xpl:remove-db/>
    </Input>
  </MustFail>

  <MustFail name="fail/remove-db-bad-name">
    <Input>
      <xpl:remove-db name="#"/>
    </Input>
  </MustFail>

  <MustFail name="fail/remove-db-bad-ignore-if-not-exists">
    <Input>
      <xpl:remove-db name="db" ignoreifnotexists="on-sundays"/>
    </Input>
  </MustFail>

  <MustFail name="fail/remove-db-access-denied">
    <Input>
      <xpl:remove-db name="db"/>
    </Input>
  </MustFail>

  <MustFail name="fail/remove-db-not-exists">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:remove-db name="heffalump"/>
    </Input>
  </MustFail>

  <Summary/>
</Root>