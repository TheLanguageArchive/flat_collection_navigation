<?php
/**
 * @author  Ibrahim Abdullah
 * @package Flat Collection Navigation
 */
class NavigationLinksGenerator
{
    /**
     * @var AbstractObject
     */
    private $object;

    /**
     * @var FedoraRepository
     */
    private $repository;

    /**
     * Constructor
     *
     * @param AbstractObject $object
     */
    public function __construct(AbstractObject $object)
    {
        $this->object     = $object;
        $this->repository = islandora_get_tuque_connection()->repository;
    }

    /**
     * Get sibling pids
     *
     * @return array
     */
    public function getSiblings(): array
    {
        $query = '
            select ?object from <#ri> WHERE
            {
                {
                    ?object <fedora-rels-ext:isMemberOfCollection> <info:fedora/' . $this->object->id . '>
                }
                ?object <fedora-model:hasModel> <info:fedora/islandora:sp_cmdiCModel> .
                ?object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>
            }
        ';

        $siblings = [];

        try {

            $objects  = $this->repository->ri->sparqlQuery($query, 10);

            foreach ($objects as $object) {
                $siblings[] = $object['object']['value'];
            }

        } catch (RepositoryException $e) {}

        return $siblings;
    }

    /**
     * Generating navigation links
     *
     * @return array
     */
    public function generate($current): array
    {
        $siblings = $this->getSiblings();
        $position = array_search($current, $siblings);
        $previous = false;
        $next     = false;

        if (count($siblings) === 0 || false === $position) {
            return [];
        }

        while (key($siblings) !== $position) {
            next($siblings);
        }

        if ($position > 0) {

            $previous = prev($siblings);
            next($siblings);
        }

        $next  = next($siblings);
        $links = [];

        return [
            'previous' => $this->generateLink('Previous', $previous),
            'next'     => $this->generateLink('Next', $next),
        ];
    }

    /**
     * Generate link to islandora object
     *
     * @param string       $label
     * @param string|false $pid
     *
     * @return string
     */
    public function generateLink(string $label, $pid = false): string
    {
        return (false === $pid ? $label : l($label, 'islandora/object/' . $pid));
    }

    /**
     * Parents are saved in an array in $variables
     * determine closest parent and return new instance
     *
     * @param array $parents
     *
     * @return NavigationLinks
     * @throws \Exception
     */
    public static function create(array &$parents): self
    {
        if (count($parents) === 0) {
            throw new \Exception('Could not find parent');
        }

        return new self($parents[0]);
    }
}
