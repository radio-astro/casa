# Note the assumption here is that the script is run in the following
# way because the argument count starts from casapy not the script name
#
# casapy --nogui --nologger -c runrecipe.py <recipe> <vislist>
#


# The system module
import sys

# Get the recipe module and the recipe name
recipe_name = sys.argv[sys.argv.index('-c')+2]
recipe_module_name = 'pipeline.recipes.'+recipe_name

# Get the recipe
recipe_module = __import__(recipe_module_name, fromlist=[recipe_name])
recipe = getattr(recipe_module, recipe_name)

# Execute the request
datasets=sys.argv[sys.argv.index('-c')+3].split(',')
recipe(datasets, importonly=False)
