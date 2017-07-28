import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
import numpy as np

import os
import commands

import pickle

# Run simulation and plot simulation statistics
def test(N, t, deltar, costr, policy, V):
    output_dir = 'output/sim/'
    sim_command = './test t=' + str(t) + ' deltar=' + str(deltar) + ' costr=' + str(costr)
    sim_command += ' policy=' + policy + ' V=' + str(V) + ' logging=false'
    sim_instance = 'N_' + str(N) + '_t_' + str(t) + '_deltar_' + str(deltar) + '_costr_' + str(costr)
    sim_instance += '_' + policy + '_V_' + str(V)

    commands.getstatusoutput(sim_command)    
    
    schedule = pd.DataFrame.from_csv(output_dir + 'schedule_' + sim_instance + '.csv')
    queue = pd.DataFrame.from_csv(output_dir + 'queue_' + sim_instance + '.csv')
    cost = pd.DataFrame.from_csv(output_dir + 'cost_' + sim_instance + '.csv')
    
    node_rate_columns = schedule.columns[[('rate' in x and 'node' in x) for x in schedule.columns]]
    link_rate_columns = schedule.columns[[('rate' in x and 'link' in x) for x in schedule.columns]]
    
    """
    plt.figure()
    fig, axes = plt.subplots(1, 4, figsize=(15, 4));
    schedule[node_rate_columns].mean().plot(kind='bar', ylim=[0,4], ax=axes[0])
    plt.title('V = ' + str(V))
    
    schedule[link_rate_columns].mean().plot(kind='bar', ylim=[0,4], ax=axes[1])
    
    queue.sum(axis=1).plot(ax=axes[2])
    
    cost.sum(axis=1).plot(ax=axes[3])
    """
    
    print 'Mean queue length sum = ', queue.sum(axis=1)[(t/10):].mean()
    print 'Mean cost = ', cost.sum(axis=1)[(t/10):].mean()

    commands.getstatusoutput('rm ' + output_dir + 'schedule_' + sim_instance + '.csv')
    commands.getstatusoutput('rm ' + output_dir + 'queue_' + sim_instance + '.csv')
    commands.getstatusoutput('rm ' + output_dir + 'cost_' + sim_instance + '.csv')


    return schedule, queue, cost



# ## Adaptive DCNC with various parameter V

# For the following simulations, we fix the duration of reconfiguration delay as $\Delta_r = 10$.
# For each V, the subfigures indicate 
# (a) mean process rate at each node (b) mean transmission rate at each link (c) evoluation of total queue length

# In[6]:

N = 6
t = 300000
deltar = 10
costr = 0
V = 10.0
policy = 'DCNC'


# In[11]:

deltars = [0, 5, 10]
deltar_costs = []
deltar_queues = []
for deltar in deltars:
    costs = []
    queues = []
    Vs = [1.0, 2.0, 5.0, 10.0, 20.0, 50.0, 100.0, 200.0]
    for V in Vs:
        t = int(100000 * (3**(np.floor(np.log10(V)))))
        print 'V = ', V, ', deltar = ', deltar
        schedule, queue, cost = test(N, t, deltar, costr, policy, V)
        costs.append(cost.sum(axis=1).mean())
        queues.append(queue.sum(axis=1).mean())
        del schedule, queue, cost
    
    deltar_costs.append(costs)
    deltar_queues.append(queues)


# #### Cost - Delay tradeoff with varying V

# In the following figures, we show the mean costs and the mean total queue lengths with varying V.
# 
# From the figures below, we could observe the cost-delay tradeoff under various paramter V.

plt.figure(figsize=(10,5))
plt.subplot(1,2,1)
for queues in deltar_queues:
    plt.plot(Vs, queues)
    plt.xlabel('V')
    plt.ylabel('Mean sum of queue lengths')
plt.subplot(1,2,2)
for costs in deltar_costs:
    plt.plot(Vs, costs)
    plt.xlabel('V')
    plt.ylabel('Mean costs')

plt.savefig('delay_cost_'+policy+'.png', bbox_inches='tight')

data = {'deltar_costs': deltar_costs, 'deltar_queues': deltar_queues, 'deltars': deltars, 'Vs': Vs}

with open('delay_cost_'+policy+'.out', 'w') as outFile:
    pickle.dump(data, outFile)

# ### Processing and transmission rate for each commodity

# We now analyze the previous simulation results to check the (processing/transmission) rate 
# of each commodity at different nodes/links.
# 
# For each node/link, commodity 0 (shown in gray color in the bar plots) indicates the time 
# portion that the node/link is in reconfiguration.

"""
def rate_plot(schedule, cost, V):
    # merge schedule and cost data
    cost.columns = [x+'_cost' for x in cost.columns]
    schedule = schedule.join(cost, how='inner')
    
    node_rates = pd.DataFrame()
    node_costs = pd.DataFrame()
    for n in xrange(N):
        node = schedule.columns[['node'+str(n) in x for x in schedule.columns]]
        df = schedule[node].copy()
        # Set the scheduled packet as NaN when in reconfiguration
        df.loc[df['node'+str(n)+'_reconfig'] > 0, 'node'+str(n)+'_packet'] = None
        # Set the process cost as 0 when in reconfiguration
        df.loc[df['node'+str(n)+'_reconfig'] > 0, 'node'+str(n)+'_px_cost'] = 0
        # Sum up the process and allocation cost
        df['node'+str(n)+'_cost'] = df['node'+str(n)+'_px_cost'] + df['node'+str(n)+'_resource_cost']
        df = df.fillna(0)
        df_rate = (df.groupby('node'+str(n)+'_packet').sum() / df.shape[0])[['node'+str(n)+'_rate']]
        df_rate.index.name = ''
        df_cost = (df.groupby('node'+str(n)+'_packet').sum() / df.shape[0])[['node'+str(n)+'_cost']]
        df_cost.index.name = ''

        if node_rates.empty:
            node_rates = df_rate
            node_costs = df_cost
        else:
            node_rates = node_rates.join(df_rate, how='outer')
            node_costs = node_costs.join(df_cost, how='outer')

    node_rates = node_rates.sort_index(ascending=False)
    node_rates = node_rates.stack()
    node_rates = node_rates.unstack(0)
    node_costs = node_costs.sort_index(ascending=False)
    node_costs = node_costs.stack()
    node_costs = node_costs.unstack(0)
    
    link_rates = pd.DataFrame()
    link_costs = pd.DataFrame()
    for l in schedule.columns[['link' in x and 'rate' in x for x in schedule.columns]].str.strip('_rate'):
        link = schedule.columns[[l in x for x in schedule.columns]]
        df = schedule[link].copy()
        # Set the scheduled packet as NaN when in reconfiguration
        df.loc[df[l+'_reconfig'] > 0, l+'_packet'] = None
        # Set the transmission cost as 0 when in reconfiguration
        df.loc[df[l+'_reconfig'] > 0, l+'_tx_cost'] = 0
        # Sum up the transmission and allocation cost
        df[l+'_cost'] = df[l+'_tx_cost'] + df[l+'_resource_cost']
        df = df.fillna(0)
        df_rate = (df.groupby(l+'_packet').sum() / df.shape[0])[[l+'_rate']]
        df_rate.index.name = ''
        df_cost = (df.groupby(l+'_packet').sum() / df.shape[0])[[l+'_cost']]
        df_cost.index.name = ''
        if link_rates.empty:
            link_rates = df_rate
            link_costs = df_cost
        else:
            link_rates = link_rates.join(df_rate, how='outer')
            link_costs = link_costs.join(df_cost, how='outer')

    link_rates = link_rates.sort_index(ascending=False)
    link_rates = link_rates.stack()
    link_rates = link_rates.unstack(0)
    link_costs = link_costs.sort_index(ascending=False)
    link_costs = link_costs.stack()
    link_costs = link_costs.unstack(0)
    
    fig, axes = plt.subplots(1, 4, figsize=(15, 5))
    fig.tight_layout(pad=6.0)
    node_rates.plot(kind='bar', stacked=True, ylim=[0,4], ax=axes[0], colormap='nipy_spectral')
    axes[0].set_title('V = ' + str(V))
    axes[0].legend(bbox_to_anchor=(1.05, 1), loc=2, borderaxespad=0.)
    link_rates.plot(kind='bar', stacked=True, ylim=[0,4], ax=axes[1], colormap='nipy_spectral')
    axes[1].set_title('V = ' + str(V))
    axes[1].legend(bbox_to_anchor=(1.05, 1), loc=2, borderaxespad=0.)
    node_costs.plot(kind='bar', stacked=True, ax=axes[2], colormap='nipy_spectral')
    axes[2].set_title('V = ' + str(V))
    axes[2].legend(bbox_to_anchor=(1.05, 1), loc=2, borderaxespad=0.)
    link_costs.plot(kind='bar', stacked=True, ax=axes[3], colormap='nipy_spectral')
    axes[3].set_title('V = ' + str(V))
    axes[3].legend(bbox_to_anchor=(1.05, 1), loc=2, borderaxespad=0.)
"""

# In[198]:

"""
for V in Vs:
    t = int(100000 * (3**(np.floor(np.log10(V)))))
    output_dir = 'output/sim/'
    sim_command = './test t=' + str(t) + ' deltar=' + str(deltar) + ' policy=' + policy + ' V=' + str(V)
    sim_instance = 'N_' + str(N) + '_t_' + str(t) + '_deltar_' + str(deltar) + '_' + policy + '_V_' + str(V)
    schedule = pd.DataFrame.from_csv(output_dir + 'schedule_' + sim_instance + '.csv')
    cost = pd.DataFrame.from_csv(output_dir + 'cost_' + sim_instance + '.csv')
    #queue = pd.DataFrame.from_csv(output_dir + 'queue_' + sim_instance + '.csv')
    
    plt.figure()
    rate_plot(schedule, cost, V)
"""


# In the bar plots above, the gray area indicate the portion when one resource is in reconfiguration. 
# In other words, during these portion, the resource is allocated but is not processing/transmitting 
# due to reconfiguration delay.
# 
# Comparing the results under different parameter V, we may observe that larger V parameter induces 
# less variation in the route selection and the NFV location, and in turn reduces the overhead wasted 
# to reconfiguration. However, this is at the expense of larger queue length (delay), as indicated in 
# the cost-delay tradeoff shown above.



