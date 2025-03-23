from django.contrib import admin
from .models import Noeud, Arete, Vehicule, DemandeLivraison

admin.site.register(Noeud)
admin.site.register(Arete)
admin.site.register(Vehicule)
admin.site.register(DemandeLivraison)
