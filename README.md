# mod-auctionator

This mod is meant to keep a healthy auction house stocked on a low-pop server. It's in it's early phases of building/testing/configuration but keeps a LOT of stuff in the AH. Code is also a little rough right now but I haven't done c++ in 20+ years so I am getting there.

## Installation and Setup

1. Download the code into your source code `modules/` folder
2. Do the Compilation Things as best you can.
3. Run the database script in `mod-auctionator/data/sql/db-world/base/2023-09-18.sql`
4. If you want a fresh start clear you auction house with these SQL commands:

```
DELETE FROM acore_world.mod_acore_characters.item_instance ii WHERE ii.guid IN (
	SELECT ah.itemguid
	FROM acore_world.mod_acore_characters.auctionhouse ah
);

DELETE FROM acore_world.mod_acore_characters.auctionhouse;
```

## Current limitations

1. Caps at a hardcoded 10000 items in the AH (you can edit and recompile).
2. No control on stack size, it is hard coded to 20 (you can edit and recompile).
3. No control over item ranges (you can change the query).