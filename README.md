GameInventorySystemPlugin
=========================

The API is not yet finalized, but general idea is that you can derive from UGISInventoryBaseComponent
and create new component, which can in turn store any arbitrary data.

That's why base component doesn't have any properties, which could store inventory data.

Default plugin have two componenets. ActionBar and Inventory. Names are pretty self explanatory.

Also each componene need it's own Container widget. You can write one using C++ or do it using blueprint. Important part is
that your widget should derive from UGISContainerBase. This widget will be used to store basic data, as well as
will implement client side interactions between componenets. 

Once you have your own componenets, you must decide, how they will interact between each other. You do it by
implementing AddItemOnSlot(), here you define, how does items should behave when moved within single component, or when
moved between componenets.
For example if you use component to store Abilities, you might not want to remove them from component, when moving to action bar.

The sample implementation for Inventory-ActionBar interaction should be sufficient to understrand how it could be done. (It's not
yet done).

Also all inventory related Widgets, should derive from base C++ classes defined for them. GISContainerBase, GISItemBase, 
GISSlotBase.
